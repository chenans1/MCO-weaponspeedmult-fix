#pragma once

namespace weaponspeedmultFix {
    class hkbHook {
    public:
        static void Install();

    private:
        static void Activate(RE::hkbClipGenerator* self, const RE::hkbContext& a_context);
        static inline REL::Relocation<decltype(Activate)> _originalActivate;
    };
}