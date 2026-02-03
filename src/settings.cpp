#include "PCH.h"
#include "settings.h"
#include <SimpleIni.h>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

static bool ini_bool(CSimpleIniA& ini, const char* section, const char* key, bool def) {
    return ini.GetLongValue(section, key, def ? 1L : 0L) != 0;
}

namespace settings {

    static config cfg{};
    config& Get() { return cfg; }

    void load() {
        constexpr auto path = "Data/SKSE/Plugins/wsmFix.ini";
        CSimpleIniA ini;

        ini.SetUnicode(false);
        const SI_Error rc = ini.LoadFile(path);

        if (rc < 0) {
            // File missing or unreadable. Keep defaults in g_cfg.
            log::warn("Could not load ini '{}'. Using defaults.", path);
            return;
        }

        auto& c = Get();
        c.enableLog = ini_bool(ini, "general", "enableLog", c.enableLog);
        c.enableWeaponSpeed = ini_bool(ini, "general", "enableWeaponSpeed", c.enableWeaponSpeed);
        c.avgDW = ini_bool(ini, "general", "averageDualWield", c.avgDW);
        SKSE::log::info("Settings loaded: log={}, applyWeaponSpeed={}, averageDualWield={}", c.enableLog, c.enableWeaponSpeed, c.avgDW);
    }
}