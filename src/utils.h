#pragma once
#include <cmath>

namespace utils {
    //check for lowercase equality
    inline bool ieq(char a, char b) noexcept {
        return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
    }

    inline float normWSM(float wsm) noexcept {
        /*guard against 0/negative weaponspeedmult values (IIRC this is fixed by noahboddie's attackspeed SKSE patch)
        but just in case:*/
        if (wsm == 0.0f || wsm == 1.0f) {
            return 1.0f;
        }
        if (!std::isfinite(wsm) || wsm < 0.0f) {
            return 1.0f;
        }
        return wsm;
    }

    bool istarts_with(std::string_view s, std::string_view prefix) noexcept;
    std::string_view basename_view(std::string_view path) noexcept;
}