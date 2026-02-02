#pragma once

namespace utils {
    //check for lowercase equality
    inline bool ieq(char a, char b) noexcept {
        return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
    }

    bool istarts_with(std::string_view s, std::string_view prefix) noexcept;
    std::string_view basename_view(std::string_view path) noexcept;
}