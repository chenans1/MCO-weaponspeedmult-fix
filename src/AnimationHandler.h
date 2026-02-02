#pragma once

namespace weaponspeedmultFix {
    class hkbHook {
    public:
        static void Install();

    private:
        static void Activate(RE::hkbClipGenerator* self, const RE::hkbContext& a_context);
        static inline REL::Relocation<decltype(Activate)> _originalActivate;

        static void Update(RE::hkbClipGenerator* self, const RE::hkbContext& a_context, float a_timestep);
        static inline REL::Relocation<decltype(Update)> _originalUpdate;

    };
}