#include "PCH.h"
#include "AnimationHandler.h"
#include "utils.h"
#include <array>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace weaponspeedmultFix {
    static bool IsBFCOAttackClip(std::string_view clipPath) noexcept {
        static constexpr std::array<std::string_view, 5> kPrefixes = {
            "BFCO_attack", "BFCO_powerattack", "BFCO_SpecialAttack", "BFCO_SpecialAttackPower", "MCO_weaponart"};
        const auto file = utils::basename_view(clipPath);
        for (auto p : kPrefixes) {
            if (utils::istarts_with(file, p)) return true;
        }
        return false;
    }

    static bool IsMCOAttackClip(std::string_view clipPath) noexcept {
        static constexpr std::array<std::string_view, 5> kPrefixes = {
            "MCO_attack", "MCO_powerattack", "MCO_sprintattack", "MCO_sprintpowerattack", "MCO_weaponart"};

        const auto file = utils::basename_view(clipPath);
        for (auto p : kPrefixes) {
            if (utils::istarts_with(file, p)) return true;
        }
        return false;
    }

    //test from asrak's magicutils: -0xC0 pointer offset
    [[nodiscard]] inline RE::BShkbAnimationGraph* GraphFromCharacter(RE::hkbCharacter* chr) noexcept {
        if (!chr) {
            return nullptr;
        }
        //from BShkbAnimationGraph.h: hkbCharacter characterInstance;// 0C0
        return SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(chr, -0xC0);
    }

    void hkbHook::Install() { 
        log::info("[hkbHook]: attempting to install hooks");
        REL::Relocation<std::uintptr_t> vtblhkbClipGenerator{RE::VTABLE_hkbClipGenerator[0]};
        /*_originalActivate = vtblhkbClipGenerator.write_vfunc(0x4, Activate);
        log::info("[hkbHook]: installed Activate hook at slot 0x4");*/

        _originalUpdate = vtblhkbClipGenerator.write_vfunc(0x05, &hkbHook::Update);
        log::info("[hkbHook]: installed Update hook at slot 0x5");

    }

    void hkbHook::Activate(RE::hkbClipGenerator* self, const RE::hkbContext& a_context) {
        if (!self) {
            log::warn("[hkbHook::Activate]: no self");
            return;
        }
        _originalActivate(self, a_context);

        const char* name = self->animationName.c_str();
        /*log::info("[hkbClipGenerator::Activate] clip='{}' speed={}", name ? name : "<null>", self->playbackSpeed);*/
        if (IsMCOAttackClip(name)) {
            log::info("[hkbHook::Activate] clip='{}' speed={}", name ? name : "<null>",
                        self->playbackSpeed);
        }   
    }

    void hkbHook::Update(RE::hkbClipGenerator* self, const RE::hkbContext& a_context, float a_timestep) {
        if (!self) {
            log::warn("[hkbHook::Update] no self");
            return _originalUpdate(self, a_context, a_timestep);
        }

        const char* raw = self->animationName.c_str();
        if (!raw || !IsMCOAttackClip(raw)) {
            return _originalUpdate(self, a_context, a_timestep);
        }
        //log::info("[hkbHook::Update] clip='{}' speed={}", raw ? raw : "<null>", self->playbackSpeed);
        auto* graph = GraphFromCharacter(a_context.character);
        if (!graph) {
            log::warn("[hkbHook::Update]: No graph");
            return _originalUpdate(self, a_context, a_timestep);
        }
        if (graph->behaviorGraph != a_context.behavior) {
            return _originalUpdate(self, a_context, a_timestep);
        }
        
        auto* actor = graph->holder;
        if (!actor) {
            log::warn("[hkbHook::Update]: No actor");
            return _originalUpdate(self, a_context, a_timestep);
        }

        // fetch weapon speed mult actor value
        const float wsm = utils::normWSM(actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kWeaponSpeedMult));
        const float base = self->playbackSpeed;
        self->playbackSpeed = base * wsm;
        log::info("[hkbHook::Update] clip='{}', base={}, weaponspeedmult={}, updated speed={}",
            raw ? raw : "<null>", base,  wsm, self->playbackSpeed);

        //call original function
        return _originalUpdate(self, a_context, a_timestep);
    }
}