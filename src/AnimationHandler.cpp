#include "PCH.h"
#include "AnimationHandler.h"

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace weaponspeedmultFix {
    void hkbHook::Install() { 
        log::info("[hkbHook]: attempt to install hooks");
        REL::Relocation<std::uintptr_t> vtblhkbClipGenerator{RE::VTABLE_hkbClipGenerator[0]};
        _originalActivate = vtblhkbClipGenerator.write_vfunc(0x4, Activate);
        log::info("[hkbHook]: installed Activate hook at slot 0x4");
    }

    void hkbHook::Activate(RE::hkbClipGenerator* self, const RE::hkbContext& a_context) {
        if (self) {
            const char* name = self->animationName.c_str();
            log::info("[hkbClipGenerator::Activate] clip='{}' speed={}", name ? name : "<null>", self->playbackSpeed);
        }
        _originalActivate(self, a_context);
    }
}