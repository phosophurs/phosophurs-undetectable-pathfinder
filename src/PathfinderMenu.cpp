#include "PathfinderMenu.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {
    constexpr float MENU_W = 540.f;
    constexpr float MENU_H = 360.f;
    bool g_menuOpen = false;

    std::string boolText(bool value) {
        return value ? "ON" : "OFF";
    }

    std::string intText(int value) {
        return std::to_string(value);
    }

    std::string floatText(double value) {
        return fmt::format("{:.1f}", value);
    }
}

PhosophursMenu* PhosophursMenu::create() {
    auto ret = new PhosophursMenu();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PhosophursMenu::init() {
    if (g_menuOpen) return false;
    if (!Popup::init(MENU_W, MENU_H)) return false;
    g_menuOpen = true;

    auto title = CCLabelBMFont::create("PHOSOPHURS // PATHFINDER", "bigFont.fnt");
    title->setScale(0.48f);
    title->setAnchorPoint({0.f, 0.5f});
    title->setPosition(24.f, MENU_H - 25.f);
    this->addChild(title);

    auto version = CCLabelBMFont::create("v0.3.0  |  EXPERIMENTAL", "goldFont.fnt");
    version->setScale(0.32f);
    version->setAnchorPoint({1.f, 0.5f});
    version->setPosition(MENU_W - 22.f, MENU_H - 25.f);
    this->addChild(version);

    auto divider = CCLayerColor::create({65, 76, 100, 255}, MENU_W - 40.f, 1.f);
    divider->setPosition(20.f, MENU_H - 48.f);
    this->addChild(divider);

    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition(0.f, 0.f);
    this->addChild(m_tabMenu);

    m_page = CCNode::create();
    m_page->setPosition(0.f, 0.f);
    this->addChild(m_page);

    buildTabs();
    selectTab(0);
    return true;
}

void PhosophursMenu::buildTabs() {
    static std::array<char const*, 5> labels = {
        "AUTOPLAY", "PATHFINDER", "SOUNDS", "VISUALS", "SETTINGS"
    };
    const float startX = 76.f;
    const float gap = 97.f;
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        auto sprite = ButtonSprite::create(labels[i], 82.f, false, "goldFont.fnt", "GJ_button_04.png", 22.f, 0.34f);
        auto item = CCMenuItemExt::createSpriteExtra(sprite, [this, i](CCMenuItem*) {
            selectTab(i);
        });
        item->setPosition(startX + gap * i, MENU_H - 68.f);
        m_tabMenu->addChild(item);
    }
}

void PhosophursMenu::selectTab(int tab) {
    m_tab = std::clamp(tab, 0, 4);
    buildPage();
}

void PhosophursMenu::addSection(char const* title, float& y) {
    auto label = CCLabelBMFont::create(title, "goldFont.fnt");
    label->setScale(0.38f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition(34.f, y);
    m_page->addChild(label);
    y -= 18.f;
}

void PhosophursMenu::addInfo(char const* text, float& y) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.34f);
    const float maxWidth = MENU_W - 76.f;
    if (label->getContentSize().width * label->getScale() > maxWidth)
        label->setScale(maxWidth / label->getContentSize().width);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition(38.f, y);
    label->setColor({190, 200, 218});
    m_page->addChild(label);
    y -= 16.f;
}

void PhosophursMenu::addAction(char const* label, std::function<void()> callback, float& y) {
    auto text = CCLabelBMFont::create(label, "bigFont.fnt");
    text->setScale(0.38f);
    const float maxLabelWidth = MENU_W - 190.f;
    if (text->getContentSize().width * text->getScale() > maxLabelWidth)
        text->setScale(maxLabelWidth / text->getContentSize().width);
    text->setAnchorPoint({0.f, 0.5f});
    text->setPosition(38.f, y);
    m_page->addChild(text);

    auto menu = CCMenu::create();
    menu->setPosition(0.f, 0.f);
    auto button = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("CHANGE", 76.f, false, "goldFont.fnt", "GJ_button_05.png", 20.f, 0.36f),
        [callback = std::move(callback)](CCMenuItem*) { callback(); }
    );
    button->setPosition(MENU_W - 82.f, y);
    menu->addChild(button);
    m_page->addChild(menu);
    y -= 21.f;
}

void PhosophursMenu::addToggle(char const* label, char const* setting, float& y) {
    auto mod = Mod::get();
    const bool enabled = mod->getSettingValue<bool>(setting);
    auto text = fmt::format("{}: {}", label, boolText(enabled));
    addAction(text.c_str(), [setting]() {
        auto m = Mod::get();
        const bool value = m->getSettingValue<bool>(setting);
        m->setSettingValue<bool>(setting, !value);
    }, y);
}

void PhosophursMenu::addCycle(char const* label, char const* setting, float& y,
                              std::vector<int> const& values,
                              std::vector<std::string> const& names) {
    auto mod = Mod::get();
    const int current = mod->getSettingValue<int>(setting);
    auto it = std::find(values.begin(), values.end(), current);
    const size_t index = it == values.end() ? 0 : static_cast<size_t>(it - values.begin());
    const std::string shown = index < names.size() ? names[index] : intText(current);
    auto text = fmt::format("{}: {}", label, shown);
    addAction(text.c_str(), [setting, values]() {
        auto m = Mod::get();
        const int old = m->getSettingValue<int>(setting);
        auto found = std::find(values.begin(), values.end(), old);
        size_t index = found == values.end() ? 0 : static_cast<size_t>(found - values.begin());
        index = (index + 1) % values.size();
        m->setSettingValue<int>(setting, values[index]);
    }, y);
}

void PhosophursMenu::addFloatCycle(char const* label, char const* setting, float& y,
                                   std::vector<double> const& values) {
    auto mod = Mod::get();
    const double current = mod->getSettingValue<double>(setting);
    auto it = std::find(values.begin(), values.end(), current);
    const size_t index = it == values.end() ? 0 : static_cast<size_t>(it - values.begin());
    auto text = fmt::format("{}: {} px", label, floatText(current));
    addAction(text.c_str(), [setting, values]() {
        auto m = Mod::get();
        const double old = m->getSettingValue<double>(setting);
        auto found = std::find(values.begin(), values.end(), old);
        size_t index = found == values.end() ? 0 : static_cast<size_t>(found - values.begin());
        index = (index + 1) % values.size();
        m->setSettingValue<double>(setting, values[index]);
    }, y);
}

void PhosophursMenu::buildPage() {
    m_page->removeAllChildrenWithCleanup(true);
    float y = MENU_H - 108.f;

    switch (m_tab) {
        case 0: {
            addSection("AUTOPLAY CONTROL", y);
            addInfo("Toggle autoplay with key 4 (configurable below).", y);
            addInfo("The automated input engine is still in development.", y);
            addSection("KEYBINDS", y);
            addAction("Toggle autoplay key", [] { openSettingsPopup(Mod::get()); }, y);
            addAction("Pause / resume key", [] { openSettingsPopup(Mod::get()); }, y);
            addAction("Recalculate path key", [] { openSettingsPopup(Mod::get()); }, y);
            addToggle("Progress-based mistakes", "progress-mistakes-enabled", y);
            addCycle("Mistake mode", "mistake-mode", y,
                     {0, 1, 2}, {"Extra clicks", "Skipped clicks", "Both"});
            addCycle("Mistake chance", "mistake-chance", y,
                     {0, 5, 10, 15, 20, 30, 50, 75, 100},
                     {"0%", "5%", "10%", "15%", "20%", "30%", "50%", "75%", "100%"});
            break;
        }
        case 1: {
            addSection("PATHFINDER OPTIONS", y);
            addToggle("Humanised mouse clicks", "humanised-clicks-enabled", y);
            addFloatCycle("Randomisation radius", "humanised-click-strength", y,
                          {0, 0.5, 1, 2, 3, 4, 6, 8, 10, 12});
            addToggle("Show predicted path", "show-predicted-path", y);
            addInfo("Frame-perfect clicks are never offset.", y);
            addInfo("Keyboard clicks are not position-randomised.", y);
            addSection("ENGINE STATUS", y);
            addInfo("Route solver: NOT IMPLEMENTED", y);
            addInfo("Mouse input scheduler: NOT IMPLEMENTED", y);
            addAction("Recalculate path key", [] { openSettingsPopup(Mod::get()); }, y);
            break;
        }
        case 2: {
            addSection("CLICK AUDIO", y);
            addToggle("Master click sounds", "click-sounds-enabled", y);
            addToggle("Keyboard click sounds", "keyboard-clicks-enabled", y);
            addToggle("Mouse click sounds", "mouse-clicks-enabled", y);
            addToggle("Random waveform variation", "waveform-variation", y);
            addInfo("Keyboard and mouse sounds use separate generated samples.", y);
            addInfo("The game input hook currently detects mouse origin heuristically.", y);
            addAction("Toggle sound key", [] { openSettingsPopup(Mod::get()); }, y);
            break;
        }
        case 3: {
            addSection("VISUAL OVERLAY", y);
            addToggle("Show predicted path", "show-predicted-path", y);
            addInfo("Path drawing will appear here when the solver is added.", y);
            addInfo("Predicted route overlay: NOT IMPLEMENTED", y);
            addInfo("Live player marker: NOT IMPLEMENTED", y);
            addInfo("Collision / hazard preview: NOT IMPLEMENTED", y);
            addAction("Toggle path display key", [] { openSettingsPopup(Mod::get()); }, y);
            break;
        }
        case 4: {
            addSection("GENERAL SETTINGS", y);
            addInfo("Phosophurs Undetectable Pathfinder", y);
            addInfo("Target: Geometry Dash 2.2081 / Geode 5.11.2", y);
            addAction("Open Geode keybind settings", [] { openSettingsPopup(Mod::get()); }, y);
            addInfo("Menu key default: Z", y);
            addInfo("Autoplay key default: 4", y);
            addInfo("This is an early, unpolished UI build.", y);
            break;
        }
    }
}

void PhosophursMenu::onExit() {
    g_menuOpen = false;
    Popup::onExit();
}
