#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ClickAudio.hpp"
#include "PathfinderMenu.hpp"

using namespace geode::prelude;

namespace {
    bool g_autoplayRequested = false;
    bool g_paused = false;
    bool g_showPath = true;

    void notify(std::string const& message) {
        Notification::create(message, NotificationIcon::Info, 2.0f)->show();
    }

    void toggleAutoplay() {
        g_autoplayRequested = !g_autoplayRequested;
        g_paused = false;
        notify(g_autoplayRequested
            ? "PHOSOPHURS: REQUEST ENABLED (ROUTE RECORDER BUILD IN PROGRESS)"
            : "PHOSOPHURS: REQUEST DISABLED");
        log::info("Phosophurs autoplay request state: {}", g_autoplayRequested);
    }

    void openMenu() {
        PhosophursMenu::toggle();
    }

    void pauseAutoplay() {
        if (!g_autoplayRequested) return;
        g_paused = !g_paused;
        notify(g_paused ? "PHOSOPHURS: REQUEST PAUSED" : "PHOSOPHURS: REQUEST RESUMED");
    }

    void recalculatePath() {
        if (!g_autoplayRequested) {
            notify("PHOSOPHURS: START AUTOPLAY FIRST");
            return;
        }
        notify("PHOSOPHURS: PATH SOLVER NOT IMPLEMENTED");
    }

    void toggleClickSounds() {
        auto mod = Mod::get();
        bool enabled = mod->getSettingValue<bool>("click-sounds-enabled");
        mod->setSettingValue<bool>("click-sounds-enabled", !enabled);
        notify(!enabled ? "PHOSOPHURS: CLICK SOUNDS ON"
                        : "PHOSOPHURS: CLICK SOUNDS OFF");
    }

    bool isMouseButtonDown() {
        return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
               (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ||
               (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
    }
}

// The player input hook is the common point reached by normal and
// mod-synthesised gameplay presses. Mouse presses are identified from the
// Windows button state; the pathfinder can also call the explicit audio
// functions when it selects a particular input device.
class $modify(PhosophursPlayer, PlayerObject) {
    bool pushButton(PlayerButton button) {
        bool result = PlayerObject::pushButton(button);
        if (result) {
            if (isMouseButtonDown()) {
                phosophurs::audio::playMouseClick();
            } else {
                phosophurs::audio::playKeyboardClick();
            }
        }
        return result;
    }
};

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
            notify(g_showPath ? "PHOSOPHURS: PATH DISPLAY PREFERENCE ON (OVERLAY NOT READY)"
                              : "PHOSOPHURS: PATH DISPLAY PREFERENCE OFF");
        });

    listenForKeybindSettingPresses("sounds-key",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) toggleClickSounds();
        });

    log::info("Phosophurs Undetectable Pathfinder loaded.");
}
