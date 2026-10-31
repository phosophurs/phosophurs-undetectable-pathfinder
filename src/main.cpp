#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace {
    bool g_autoplayRequested = false;
    bool g_paused = false;
    bool g_showPath = true;
    bool g_clickSounds = true;

    void notify(std::string const& message) {
        Notification::create(message, NotificationIcon::Info, 2.0f)->show();
    }

    void toggleAutoplay() {
        g_autoplayRequested = !g_autoplayRequested;
        g_paused = false;

        // This is the control/state scaffold. The actual level solver and safe
        // input driver will be implemented in a later development stage.
        notify(g_autoplayRequested
            ? "PHOSOPHURS: AUTOPLAY REQUESTED"
            : "PHOSOPHURS: AUTOPLAY STOPPED");
        log::info("Phosophurs autoplay requested: {}", g_autoplayRequested);
    }

    void openMenu() {
        geode::openSettingsPopup(Mod::get());
    }

    void pauseAutoplay() {
        if (!g_autoplayRequested) return;
        g_paused = !g_paused;
        notify(g_paused ? "PHOSOPHURS: PAUSED" : "PHOSOPHURS: RESUMED");
    }

    void recalculatePath() {
        if (!g_autoplayRequested) {
            notify("PHOSOPHURS: START AUTOPLAY FIRST");
            return;
        }
        notify("PHOSOPHURS: PATH RECALCULATION REQUESTED");
    }
}

$on_game(Loaded) {
    listenForKeybindSettingPresses("autoplay-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) toggleAutoplay();
        });

    listenForKeybindSettingPresses("menu-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) openMenu();
        });

    listenForKeybindSettingPresses("pause-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) pauseAutoplay();
        });

    listenForKeybindSettingPresses("recalculate-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) recalculatePath();
        });

    listenForKeybindSettingPresses("visual-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return;
            g_showPath = !g_showPath;
            notify(g_showPath ? "PHOSOPHURS: PATH DISPLAY ON"
                              : "PHOSOPHURS: PATH DISPLAY OFF");
        });

    listenForKeybindSettingPresses("sounds-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (!down || repeat) return;
            g_clickSounds = !g_clickSounds;
            notify(g_clickSounds ? "PHOSOPHURS: CLICK SOUNDS ON"
                                 : "PHOSOPHURS: CLICK SOUNDS OFF");
        });

    log::info("Phosophurs Undetectable Pathfinder loaded.");
}
