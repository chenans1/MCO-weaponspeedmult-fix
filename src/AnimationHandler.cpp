#include "PCH.h"
#include "AnimationHandler.h"
#include "utils.h"
#include "settings.h"

#include <array>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace weaponspeedmultFix {
    //static bool IsBFCOAttackClip(std::string_view clipPath) noexcept {
    //    const auto file = utils::basename_view(clipPath);
    //    /*static constexpr std::array<std::string_view, 9> kPrefixes = {
    //        "BFCO_attack", "BFCO_powerattack", 
    //        "BFCO_SpecialAttack", "BFCO_SpecialAttackPower", 
    //        "BFCO_RangeAttack", 
    //        "BFCO_SprintAttackPower","BFCO_SprintAttack",
    //        "BFCO_JumpAttack","BFCO_JumpAttackPower"};
    //    
    //    for (auto p : kPrefixes) {
    //        if (utils::istarts_with(file, p)) return true;
    //    }
    //    return false;*/
    //    if (!utils::istarts_with(file, "BFCO_")) {
    //        return false;
    //    }
    //    return true;
    //    
    //}

    //static bool IsMCOAttackClip(std::string_view clipPath) noexcept {
    //    static constexpr std::array<std::string_view, 5> kPrefixes = {
    //        "MCO_attack", "MCO_powerattack", "MCO_sprintattack", "MCO_sprintpowerattack", "MCO_weaponart"};

    //    const auto file = utils::basename_view(clipPath);
    //    for (auto p : kPrefixes) {
    //        if (utils::istarts_with(file, p)) return true;
    //    }
    //    return false;
    //}

    //handles both either MCO or BFCO
    static bool IsTargetClip(std::string_view clipPath) noexcept {
        const auto file = utils::basename_view(clipPath);
        if (utils::istarts_with(file, "MCO_")) {
            //return IsMCOAttackClip(file);
            return true;
        }
        if (utils::istarts_with(file, "BFCO_")) {
            //return IsBFCOAttackClip(file);
            return true;
        }
        return false;
    }

    //from asrak's magicutils: -0xC0 pointer offset
    [[nodiscard]] inline RE::BShkbAnimationGraph* GraphFromCharacter(RE::hkbCharacter* chr) noexcept {
        if (!chr) {
            return nullptr;
        }
        //from BShkbAnimationGraph.h: hkbCharacter characterInstance;// 0C0
        return SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(chr, -0xC0);
    }

    //check if the actor is dual wielding
    static bool IsDualWielding(const RE::Actor* actor) {
        if (!actor) {
            return false;
        }
        const auto* rightForm = actor->GetEquippedObject(false);
        const auto* leftForm = actor->GetEquippedObject(true);

        const auto* rightWeap = rightForm ? rightForm->As<RE::TESObjectWEAP>() : nullptr;
        const auto* leftWeap = leftForm ? leftForm->As<RE::TESObjectWEAP>() : nullptr;
        if (!rightWeap || !leftWeap) {
            // If left is a shield/spell/empty, leftWeap will be null -> not dual wield
            return false;
        }

        const auto rType = rightWeap->GetWeaponType();
        const auto lType = leftWeap->GetWeaponType();

        const bool rightIs1H = (rType >= RE::WeaponTypes::kOneHandSword && rType <= RE::WeaponTypes::kOneHandMace);

        const bool leftIs1H = (lType >= RE::WeaponTypes::kOneHandSword && lType <= RE::WeaponTypes::kOneHandMace);

        return rightIs1H && leftIs1H;
    }

    void hkbHook::Install() { 
        log::info("[hkbHook]: attempting to install hooks");
        REL::Relocation<std::uintptr_t> vtblhkbClipGenerator{RE::VTABLE_hkbClipGenerator[0]};
        /*_originalActivate = vtblhkbClipGenerator.write_vfunc(0x4, Activate);
        log::info("[hkbHook]: installed Activate hook at slot 0x4");*/

        _originalUpdate = vtblhkbClipGenerator.write_vfunc(0x05, &hkbHook::Update);
        log::info("[hkbHook]: installed Update hook at slot 0x5");

    }

    void hkbHook::Update(RE::hkbClipGenerator* self, const RE::hkbContext& a_context, float a_timestep) {
        if (!self) {
            //log::warn("[hkbHook::Update] no self");
            return _originalUpdate(self, a_context, a_timestep);
        }

        const char* raw = self->animationName.c_str();
        /*if (!raw || (!IsMCOAttackClip(raw) && !IsBFCOAttackClip(raw))) {
            return _originalUpdate(self, a_context, a_timestep);
        }*/
        if (!raw || !IsTargetClip(raw)) {
            return _originalUpdate(self, a_context, a_timestep);
        }
        //log::info("[hkbHook::Update] clip='{}' speed={}", raw ? raw : "<null>", self->playbackSpeed);
        auto* graph = GraphFromCharacter(a_context.character);
        if (!graph) {
            log::warn("[hkbHook::Update]: No graph");
            return _originalUpdate(self, a_context, a_timestep);
        }
        
        auto* actor = graph->holder;
        if (!actor) {
            log::warn("[hkbHook::Update]: No actor");
            return _originalUpdate(self, a_context, a_timestep);
        }
        // check whether we are using the weapon speed mult or not. if we use weaponspeedmult fetch the graph variable instead
        const float base = self->playbackSpeed;
        if (settings::applyWeaponSpeed()) {
            //fetch the value of the graph variable float instead
            float rightWSM = 1.0f;
            float leftWSM = 1.0f;

            const bool gotRight = actor->GetGraphVariableFloat("weaponSpeedMult"sv, rightWSM);
            const bool gotLeft = actor->GetGraphVariableFloat("leftWeaponSpeedMult"sv, leftWSM);

            if (gotRight) {
                float wsm = utils::normWSM(rightWSM);
                if (settings::avgDualWield() && gotLeft && IsDualWielding(actor)) {
                    const float rw = utils::normWSM(rightWSM);
                    const float lw = utils::normWSM(leftWSM);

                    wsm = 0.5f * (rw + lw);
                }
                self->playbackSpeed = base * wsm;
                if (settings::isLogOn()) {
                    log::info("[hkbHook::Update] clip='{}', base={}, rWSM={}, lWSM={}, finalWSM={}, new={}",
                              raw ? raw : "<null>", base, utils::normWSM(rightWSM), utils::normWSM(leftWSM), wsm,
                              self->playbackSpeed);
                }
            }
        } else {
            // fetch weapon speed mult actor value
            auto* actorAV = actor->AsActorValueOwner();
            // float wsm = 1.0f;
            if (!actorAV) {
                log::warn("[hkbHook::Update] no actorAV for '{}'", raw ? raw : "<null>");
                return _originalUpdate(self, a_context, a_timestep);
            }
            const float wsm = utils::normWSM(actorAV->GetActorValue(RE::ActorValue::kWeaponSpeedMult));
            self->playbackSpeed = base * wsm;
            if (settings::isLogOn()) {
                log::info("[hkbHook::Update] clip='{}', base={}, weaponspeedmult(AV)={}, new speed={}",
                          raw ? raw : "<null>", base, wsm, self->playbackSpeed);
            }
        }
        //call original function
        return _originalUpdate(self, a_context, a_timestep);
    }
}