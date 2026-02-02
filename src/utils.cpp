#include "PCH.h"
#include "utils.h"

namespace utils {
    bool istarts_with(std::string_view s, std::string_view prefix) {
        if (s.size() < prefix.size()) return false;

        for (size_t i = 0; i < prefix.size(); ++i) {
            if (!ieq(s[i], prefix[i])) return false;
        }
        return true;
    }

    //clip names appear as 'Animations\MCO_attack1.hkx' -> extract MCO_attack
    std::string_view basename_view(std::string_view path) {
        const auto p1 = path.find_last_of('\\');
        const auto p2 = path.find_last_of('/');
        const auto pos = (p1 == std::string_view::npos) ? p2 : (p2 == std::string_view::npos) ? p1 : (std::max)(p1, p2);
        return (pos == std::string_view::npos) ? path : path.substr(pos + 1);
    }
}