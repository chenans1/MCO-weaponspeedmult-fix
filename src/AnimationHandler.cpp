#include "PCH.h"
#include "AnimationHandler.h"
#include "utils.h"
#include <array>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace weaponspeedmultFix {
    static bool IsBFCOAttackClip(std::string_view clipPath) noexcept {
        const auto file = utils::basename_view(clipPath);
        return utils::istarts_with(file, "BFCO_attack") || utils::istarts_with(file, "BFCO_PowerAttack");
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

    void hkbHook::Install() { 
        log::info("[hkbHook]: attempt to install hooks");
        REL::Relocation<std::uintptr_t> vtblhkbClipGenerator{RE::VTABLE_hkbClipGenerator[0]};
        _originalActivate = vtblhkbClipGenerator.write_vfunc(0x4, Activate);
        log::info("[hkbHook]: installed Activate hook at slot 0x4");
    }

    void hkbHook::Activate(RE::hkbClipGenerator* self, const RE::hkbContext& a_context) {
        if (self) {
            const char* name = self->animationName.c_str();
            /*log::info("[hkbClipGenerator::Activate] clip='{}' speed={}", name ? name : "<null>", self->playbackSpeed);*/
            if (IsMCOAttackClip(name)) {
                log::info("[hkbClipGenerator::Activate] clip='{}' speed={}", name ? name : "<null>",
                          self->playbackSpeed);
            }
        }
        _originalActivate(self, a_context);
    }
}