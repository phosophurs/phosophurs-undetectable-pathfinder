#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cmath>
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

    // Immutable snapshot of the level's placed geometry. This is the input
    // map for the predictive planner; it is deliberately separate from the
    // recorded route so a future solver can reason about unseen sections.
    struct LevelObjectSnapshot {
        int objectID;
        cocos2d::CCRect bounds;
    };
    std::vector<LevelObjectSnapshot> g_levelGeometry;
    size_t g_lastScannedObjectCount = 0;
    float g_speedPerFrame = 0.f;
    float g_jumpImpulse = 0.f;
    float g_gravityPerFrame = 0.9f;
    float g_lastPlayerX = 0.f;
    bool g_havePlayerSample = false;
    constexpr double SIM_FRAME_SECONDS = 1.0 / 60.0;

    bool isKnownHazard(int id) {
        // Standard spike variants and common sawblade IDs. Unknown custom
        // objects are intentionally not guessed to be hazards.
        switch (id) {
            case 8: case 39: case 103: case 392:
            case 88: case 89: case 216: case 217:
                return true;
            default:
                return false;
        }
    }

    struct PlannedJump {
        double time;
        double releaseTime;
    };

    bool trajectoryClearsHazard(
        cocos2d::CCPoint start,
        float playerWidth,
        float playerHeight,
        cocos2d::CCRect hazard,
        float speed,
        float gravity,
        float impulse,
        int launchDelay,
        int maxFrames
    ) {
        float x = start.x;
        float y = start.y;
        float vy = 0.f;
        bool jumped = false;
        bool landed = false;
        const float groundY = start.y;
        for (int frame = 0; frame < maxFrames; ++frame) {
            if (!jumped && frame == launchDelay) {
                vy = impulse;
                jumped = true;
            }
            if (jumped && !landed) {
                vy -= gravity;
                y += vy;
                if (y <= groundY && vy < 0.f) {
                    y = groundY;
                    vy = 0.f;
                    landed = true;
                }
            }
            x += speed;
            cocos2d::CCRect playerRect(
                x - playerWidth * 0.5f,
                y - playerHeight * 0.5f,
                playerWidth,
                playerHeight
            );
            if (playerRect.intersectsRect(hazard)) return false;
            if (x > hazard.getMaxX() + playerWidth + 8.f) return true;
        }
        return false;
    }

    bool buildPhysicsRoute(PlayLayer* layer) {
        if (!layer || !layer->m_player1 || g_levelGeometry.empty()) return false;
        auto player = layer->m_player1;
        if (player->m_playerMode != 0) {
            notify("PHOSOPHURS: EXPERIMENTAL PLANNER CURRENTLY SUPPORTS CUBE MODE ONLY");
            return false;
        }
        if (g_speedPerFrame <= 0.05f || g_jumpImpulse <= 0.1f) {
            notify("PHOSOPHURS: PLAY A SHORT CUBE RUN FIRST TO CALIBRATE SPEED AND JUMP");
            return false;
        }

        const auto playerRect = player->getObjectRect();
        const float width = playerRect.size.width;
        const float height = playerRect.size.height;
        const auto start = player->getPosition();
        const float gravity = std::clamp(g_gravityPerFrame, 0.05f, 4.0f);
        const float impulse = std::clamp(g_jumpImpulse, 2.0f, 20.0f);

        std::vector<cocos2d::CCRect> hazards;
        for (const auto& object : g_levelGeometry) {
            if (!isKnownHazard(object.objectID)) continue;
            if (object.bounds.getMaxX() <= start.x + width * 0.25f) continue;
            hazards.push_back(object.bounds);
        }
        std::sort(hazards.begin(), hazards.end(),
            [](const auto& a, const auto& b) { return a.getMinX() < b.getMinX(); });

        if (hazards.empty()) {
            notify("PHOSOPHURS: NO SUPPORTED SPIKES OR SAWS FOUND AHEAD");
            return false;
        }

        std::vector<PlannedJump> jumps;
        float simulatedX = start.x;
        double simulatedTime = layer->m_gameState.m_levelTime;
        for (const auto& hazard : hazards) {
            if (hazard.getMaxX() < simulatedX) continue;
            const float distance = hazard.getMinX() - simulatedX;
            const int framesUntilHazard = std::max(0,
                static_cast<int>(std::floor(distance / g_speedPerFrame)));
            bool found = false;
            int chosenDelay = 0;
            const int searchLimit = std::min(framesUntilHazard, 90);

            for (int delay = std::max(0, framesUntilHazard - searchLimit);
                 delay <= framesUntilHazard; ++delay) {
                const int simulationFrames = framesUntilHazard - delay +
                    static_cast<int>(std::ceil((hazard.size.width + width * 2.f) / g_speedPerFrame)) + 80;
                if (trajectoryClearsHazard(
                    {simulatedX, start.y}, width, height, hazard,
                    g_speedPerFrame, gravity, impulse, delay, simulationFrames
                )) {
                    chosenDelay = delay;
                    found = true;
                    break;
                }
            }

            if (!found) {
                notify(fmt::format("PHOSOPHURS: NO SAFE CUBE JUMP FOUND NEAR X={:.0f}", hazard.getMinX()));
                return false;
            }

            const double jumpTime = simulatedTime + chosenDelay * SIM_FRAME_SECONDS;
            jumps.push_back({jumpTime, jumpTime + SIM_FRAME_SECONDS});
            const int passFrames = chosenDelay +
                static_cast<int>(std::ceil((hazard.size.width + width * 2.f) / g_speedPerFrame)) + 2;
            simulatedX += g_speedPerFrame * passFrames;
            simulatedTime += passFrames * SIM_FRAME_SECONDS;
        }

        g_route.clear();
        for (const auto& jump : jumps) {
            g_route.push_back({jump.time, true, 1, true});
            g_route.push_back({jump.releaseTime, false, 1, true});
        }
        std::sort(g_route.begin(), g_route.end(),
            [](const auto& a, const auto& b) { return a.time < b.time; });
        g_recording = false;
        g_playbackActive = false;
        notify(fmt::format("PHOSOPHURS: BUILT EXPERIMENTAL ROUTE WITH {} JUMPS", jumps.size()));
        return true;
    }

    void scanLevelGeometry(PlayLayer* layer) {
        g_levelGeometry.clear();
        if (!layer || !layer->m_objects) return;

        const unsigned int count = layer->m_objects->count();
        g_levelGeometry.reserve(count);
        for (unsigned int i = 0; i < count; ++i) {
            auto object = static_cast<GameObject*>(layer->m_objects->objectAtIndex(i));
            if (!object || object->m_isDisabled || object->m_isDecoration) continue;
            const auto bounds = object->getObjectRect();
            g_levelGeometry.push_back({object->m_objectID, bounds});
        }
        g_lastScannedObjectCount = g_levelGeometry.size();
    }

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
        auto layer = PlayLayer::get();
        if (!layer) {
            notify("PHOSOPHURS: ENTER A LEVEL BEFORE SCANNING");
            return;
        }
        if (layer->m_gameState.m_levelTime > 0.20) {
            notify("PHOSOPHURS: RESTART FIRST - PLAN FROM THE LEVEL START");
            return;
        }

        scanLevelGeometry(layer);
        if (g_levelGeometry.empty()) {
            notify("PHOSOPHURS: LEVEL SCAN FOUND NO COLLISION OBJECTS");
            return;
        }

        if (buildPhysicsRoute(layer)) return;
        notify(fmt::format("PHOSOPHURS: SCANNED {} OBJECTS; NO ROUTE GENERATED",
            g_lastScannedObjectCount));
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

// Record real gameplay button events and replay them through Geometry Dash's
// own input handler, keeping press and release timing intact.
class $modify(PhosophursGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto layer = PlayLayer::get();
        bool wasGrounded = layer && layer->m_player1 && layer->m_player1->m_isOnGround;
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
        if (layer && g_recording && !g_dispatchingReplayInput) {
            g_route.push_back({layer->m_gameState.m_levelTime, down, button, isPlayer1});
            if (down && wasGrounded && layer->m_player1) {
                const float measured = static_cast<float>(std::abs(layer->m_player1->m_yVelocity));
                if (measured > 1.f && measured < 25.f) g_jumpImpulse = measured;
            }
        }
    }
};

class $modify(PhosophursPlayLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!this->m_player1) return;

        const float currentX = this->m_player1->getPositionX();
        if (g_havePlayerSample && dt > 0.f) {
            const float dx = currentX - g_lastPlayerX;
            if (dx > 0.f && dx < 40.f) {
                const float measuredPerFrame = dx * (dt * 60.f);
                g_speedPerFrame = g_speedPerFrame <= 0.f
                    ? measuredPerFrame
                    : g_speedPerFrame * 0.85f + measuredPerFrame * 0.15f;
            }
        }
        g_lastPlayerX = currentX;
        g_havePlayerSample = true;
        const float gravity = static_cast<float>(std::abs(this->m_player1->m_gravity));
        if (gravity > 0.05f && gravity < 4.f) g_gravityPerFrame = gravity;

        if (!g_playbackActive || g_paused) return;
        const double now = this->m_gameState.m_levelTime;
        while (g_replayIndex < g_route.size() && g_route[g_replayIndex].time <= now) {
            const auto event = g_route[g_replayIndex++];
            g_dispatchingReplayInput = true;
            this->handleButton(event.down, event.button, event.isPlayer1);
            g_dispatchingReplayInput = false;
        }
        if (g_replayIndex >= g_route.size()) {
            g_playbackActive = false;
            notify("PHOSOPHURS: ROUTE REPLAY FINISHED");
        }
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        const bool result = PlayLayer::init(level, useReplay, dontCreateObjects);
        if (result) {
            g_route.clear();
            g_levelGeometry.clear();
            g_lastScannedObjectCount = 0;
            // Keep calibrated movement/jump values across retries of the same
            // level so planning can run from the next attempt's start.
            g_havePlayerSample = false;
            g_replayIndex = 0;
            g_recording = true;
            g_playbackActive = false;
            g_paused = false;
            log::info("Phosophurs: recording this attempt's input route.");
        }
        return result;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (!g_playbackActive) {
            g_recording = false;
            if (!g_route.empty()) {
                notify(fmt::format("PHOSOPHURS: RECORDED {} INPUT EVENTS", g_route.size()));
            }
        }
        PlayLayer::destroyPlayer(player, object);
    }

    void levelComplete() {
        g_recording = false;
        g_playbackActive = false;
        PlayLayer::levelComplete();
    }

    void onQuit() {
        g_recording = false;
        g_playbackActive = false;
        g_route.clear();
        g_replayIndex = 0;
        PlayLayer::onQuit();
    }
};

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
