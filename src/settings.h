#pragma once

namespace settings {
    struct config {
        bool enableLog = false;
        bool enableWeaponSpeed = true;
        bool avgDW = true;
    };

    config& Get();

    void load();

    inline bool isLogOn() { return Get().enableLog; }
    inline bool applyWeaponSpeed() { return Get().enableWeaponSpeed; }
    inline bool avgDualWield() { return Get().avgDW; }
}