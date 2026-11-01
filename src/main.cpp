#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <vector>
#include <cstddef>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ClickAudio.hpp"
#include "PathfinderMenu.hpp"

using namespace geode::prelude;

namespace {
    struct RecordedInput {
        double time;
        bool down;
        int button;
        bool isPlayer1;
    };
    std::vector<RecordedInput> g_route;
    size_t g_replayIndex = 0;
    bool g_recording = false;
    bool g_playbackActive = false;
    bool g_dispatchingReplayInput = false;
    bool g_paused = false;
    bool g_showPath = true;

    void notify(std::string const& message) {
        Notification::create(message, NotificationIcon::Info, 2.0f)->show();
    }

    void toggleAutoplay() {
        auto layer = PlayLayer::get();
        if (!layer) {
            notify("PHOSOPHURS: ENTER A LEVEL FIRST");
            return;
        }
        if (g_route.empty()) {
            notify("PHOSOPHURS: NO ROUTE YET - PLAY A RUN TO RECORD ONE");
            return;
        }
        if (layer->m_gameState.m_levelTime > 0.20) {
            notify("PHOSOPHURS: RESTART THE LEVEL, THEN PRESS 4");
            return;
        }
        g_replayIndex = 0;
        g_playbackActive = !g_playbackActive;
        g_recording = false;
        g_paused = false;
        notify(g_playbackActive
            ? fmt::format("PHOSOPHURS: REPLAYING {} INPUT EVENTS", g_route.size())
            : "PHOSOPHURS: REPLAY STOPPED");
    }

    void openMenu() {
        PhosophursMenu::toggle();
    }

    void pauseAutoplay() {
        if (!g_playbackActive) return;
        g_paused = !g_paused;
        notify(g_paused ? "PHOSOPHURS: REPLAY PAUSED" : "PHOSOPHURS: REPLAY RESUMED");
    }

    void recalculatePath() {
        notify("PHOSOPHURS: ROUTE SOLVER IS THE NEXT BUILD STAGE");
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
