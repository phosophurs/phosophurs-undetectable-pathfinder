#include "PathfinderMenu.hpp"
#include "ClickAudio.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>
using namespace geode::prelude;

namespace {
    constexpr float MENU_W = 600.f;
    constexpr float MENU_H = 400.f;
    bool g_menuOpen = false;
    PhosophursMenu* g_menuInstance = nullptr;
    std::string boolText(bool value) { return value ? "ON" : "OFF"; }
    std::string floatText(double value) { return fmt::format("{:.1f}", value); }
    void notifySetting(char const* label, bool enabled) {
        Notification::create(fmt::format("{}: {}", label, enabled ? "ON" : "OFF"), NotificationIcon::Info, 1.4f)->show();
    }
}

PhosophursMenu* PhosophursMenu::create() {
    auto ret = new PhosophursMenu();
    if (ret->init()) { ret->autorelease(); return ret; }
    delete ret;
    return nullptr;
}

bool PhosophursMenu::init() {
    if (g_menuOpen) return false;
    if (!Popup::init(MENU_W, MENU_H)) return false;
    g_menuOpen = true;
    g_menuInstance = this;
    this->setScale(0.82f);

    auto title = CCLabelBMFont::create("PHOSOPHURS // PATHFINDER", "bigFont.fnt");
    title->setScale(0.48f);
    title->setAnchorPoint({0.f, 0.5f});
    title->setPosition(25.f, MENU_H - 25.f);
    this->addChild(title);

    auto version = CCLabelBMFont::create("v0.5.0  |  EXPERIMENTAL", "goldFont.fnt");
    version->setScale(0.34f);
    version->setAnchorPoint({1.f, 0.5f});
    version->setPosition(MENU_W - 25.f, MENU_H - 25.f);
    this->addChild(version);

    auto headerLine = CCLayerColor::create({72, 91, 125, 255}, MENU_W - 42.f, 1.f);
    headerLine->setPosition(21.f, MENU_H - 48.f);
    this->addChild(headerLine);
    auto tabRail = CCLayerColor::create({20, 28, 43, 230}, MENU_W - 36.f, 37.f);
    tabRail->setPosition(18.f, MENU_H - 91.f);
    this->addChild(tabRail);
    auto contentPanel = CCLayerColor::create({17, 23, 36, 220}, MENU_W - 36.f, MENU_H - 111.f);
    contentPanel->setPosition(18.f, 17.f);
    this->addChild(contentPanel);

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
    static std::array<char const*, 5> labels = {"AUTOPLAY", "PATHFINDER", "SOUNDS", "VISUALS", "SETTINGS"};
    const float startX = 78.f, gap = 111.f;
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        auto sprite = ButtonSprite::create(labels[i], 94.f, false, "goldFont.fnt",
            i == m_tab ? "GJ_button_02.png" : "GJ_button_04.png", 22.f, 0.34f);
        auto item = CCMenuItemExt::createSpriteExtra(sprite, [this, i](CCMenuItem*) { selectTab(i); });
        item->setPosition(startX + gap * i, MENU_H - 72.f);
        m_tabMenu->addChild(item);
    }
}

void PhosophursMenu::selectTab(int tab) {
    m_tab = std::clamp(tab, 0, 4);
    m_tabMenu->removeAllChildrenWithCleanup(true);
    buildTabs();
    buildPage();
}

void PhosophursMenu::addSection(char const* title, float& y) {
    auto label = CCLabelBMFont::create(title, "goldFont.fnt");
    label->setScale(0.39f);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition(34.f, y);
    m_page->addChild(label);
    auto line = CCLayerColor::create({63, 83, 115, 210}, MENU_W - 90.f, 1.f);
    line->setPosition(34.f, y - 11.f);
    m_page->addChild(line);
    y -= 22.f;
}

void PhosophursMenu::addInfo(char const* text, float& y) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.34f);
    const float maxWidth = MENU_W - 92.f;
    if (label->getContentSize().width * label->getScale() > maxWidth)
        label->setScale(maxWidth / label->getContentSize().width);
    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition(38.f, y);
    label->setColor({183, 195, 215});
    m_page->addChild(label);
    y -= 17.f;
}

void PhosophursMenu::addAction(char const* label, char const* buttonText, std::function<void()> callback, float& y) {
    auto row = CCLayerColor::create({29, 39, 57, 175}, MENU_W - 78.f, 22.f);
    row->setPosition(30.f, y - 10.5f);
    m_page->addChild(row);
    auto text = CCLabelBMFont::create(label, "bigFont.fnt");
    text->setScale(0.38f);
    const float maxWidth = MENU_W - 205.f;
    if (text->getContentSize().width * text->getScale() > maxWidth)
        text->setScale(maxWidth / text->getContentSize().width);
    text->setAnchorPoint({0.f, 0.5f});
    text->setPosition(40.f, y);
    m_page->addChild(text);
    auto menu = CCMenu::create();
    menu->setPosition(0.f, 0.f);
    auto button = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create(buttonText, 82.f, false, "goldFont.fnt", "GJ_button_05.png", 20.f, 0.36f),
        [callback = std::move(callback)](CCMenuItem*) { if (callback) callback(); });
    button->setPosition(MENU_W - 91.f, y);
    menu->addChild(button);
    m_page->addChild(menu);
    y -= 25.f;
}

void PhosophursMenu::addToggle(char const* label, char const* setting, float& y) {
    const bool enabled = Mod::get()->getSettingValue<bool>(setting);
    const auto shown = fmt::format("{}  [{}]", label, boolText(enabled));
    addAction(shown.c_str(), "TOGGLE", [setting, label]() {
        auto mod = Mod::get();
        const bool next = !mod->getSettingValue<bool>(setting);
        mod->setSettingValue<bool>(setting, next);
        notifySetting(label, next);
    }, y);
}

void PhosophursMenu::addCycle(char const* label, char const* setting, float& y,
    std::vector<int> const& values, std::vector<std::string> const& names) {
    const int current = Mod::get()->getSettingValue<int>(setting);
    auto it = std::find(values.begin(), values.end(), current);
    const size_t index = it == values.end() ? 0 : static_cast<size_t>(it - values.begin());
    const std::string shown = index < names.size() ? names[index] : std::to_string(current);
    const auto rowText = fmt::format("{}  [{}]", label, shown);
    addAction(rowText.c_str(), "NEXT", [setting, values, names, label]() {
        if (values.empty()) return;
        auto mod = Mod::get();
        const int old = mod->getSettingValue<int>(setting);
        auto found = std::find(values.begin(), values.end(), old);
        size_t next = found == values.end() ? 0 : static_cast<size_t>(found - values.begin());
        next = (next + 1) % values.size();
        mod->setSettingValue<int>(setting, values[next]);
        const auto name = next < names.size() ? names[next] : std::to_string(values[next]);
        Notification::create(fmt::format("{}: {}", label, name), NotificationIcon::Info, 1.4f)->show();
    }, y);
}

void PhosophursMenu::addFloatCycle(char const* label, char const* setting, float& y, std::vector<double> const& values) {
    const double current = Mod::get()->getSettingValue<double>(setting);
    const auto rowText = fmt::format("{}  [{} px]", label, floatText(current));
    addAction(rowText.c_str(), "NEXT", [setting, values, label]() {
        if (values.empty()) return;
        auto mod = Mod::get();
        const double old = mod->getSettingValue<double>(setting);
        auto found = std::find(values.begin(), values.end(), old);
        size_t next = found == values.end() ? 0 : static_cast<size_t>(found - values.begin());
        next = (next + 1) % values.size();
        mod->setSettingValue<double>(setting, values[next]);
        Notification::create(fmt::format("{}: {} px", label, floatText(values[next])), NotificationIcon::Info, 1.4f)->show();
    }, y);
}

void PhosophursMenu::buildPage() {
    m_page->removeAllChildrenWithCleanup(true);
    float y = MENU_H - 119.f;
    switch (m_tab) {
        case 0:
            addSection("AUTOPLAY", y);
            addInfo("Press 4 to toggle the autoplay request state.", y);
            addInfo("The automated input engine is not implemented yet.", y);
            addSection("INPUT CONTROLS", y);
            addAction("Autoplay, pause and recalculation keybinds", "SETTINGS", [] { openSettingsPopup(Mod::get()); }, y);
            addToggle("Progress-based input variation", "progress-mistakes-enabled", y);
            addCycle("Variation behaviour", "mistake-mode", y, {0,1,2}, {"Extra clicks","Skipped clicks","Both"});
            addCycle("Variation probability", "mistake-chance", y, {0,5,10,15,20,30,50,75,100}, {"0%","5%","10%","15%","20%","30%","50%","75%","100%"});
            addCycle("Variation start", "progress-start", y, {5,10,20,30,40,50,60,70,80,90,100}, {"5%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"});
            break;
        case 1:
            addSection("PATHFINDER OPTIONS", y);
            addToggle("Humanised mouse-click offsets", "humanised-clicks-enabled", y);
            addFloatCycle("Randomisation radius", "humanised-click-strength", y, {0,0.5,1,2,3,4,6,8,10,12});
            addToggle("Show predicted path", "show-predicted-path", y);
            addInfo("Frame-perfect inputs always receive zero offset.", y);
            addInfo("Keyboard inputs are never position-randomised.", y);
            addSection("ENGINE STATUS", y);
            addInfo("Route solver: NOT IMPLEMENTED", y);
            addInfo("Automated input scheduler: NOT IMPLEMENTED", y);
            addAction("Recalculate keybind", "SETTINGS", [] { openSettingsPopup(Mod::get()); }, y);
            break;
        case 2:
            addSection("CLICK AUDIO", y);
            addToggle("Master click sounds", "click-sounds-enabled", y);
            addToggle("Keyboard click sounds", "keyboard-clicks-enabled", y);
            addToggle("Mouse click sounds", "mouse-clicks-enabled", y);
            addToggle("Waveform variation", "waveform-variation", y);
            addInfo("Keyboard and mouse use separate generated WAV samples.", y);
            addInfo("Input-source detection is heuristic, not device-perfect.", y);
            addAction("Click-sound keybind", "SETTINGS", [] { openSettingsPopup(Mod::get()); }, y);
            addAction("Preview keyboard click", "PLAY", [] { phosophurs::audio::playKeyboardClick(); }, y);
            addAction("Preview mouse click", "PLAY", [] { phosophurs::audio::playMouseClick(); }, y);
            break;
        case 3:
            addSection("VISUAL OVERLAY", y);
            addToggle("Show predicted path", "show-predicted-path", y);
            addInfo("Predicted route overlay: NOT IMPLEMENTED", y);
            addInfo("Live player marker: NOT IMPLEMENTED", y);
            addInfo("Collision and hazard preview: NOT IMPLEMENTED", y);
            addAction("Path display keybind", "SETTINGS", [] { openSettingsPopup(Mod::get()); }, y);
            break;
        case 4:
            addSection("GENERAL SETTINGS", y);
            addInfo("Phosophurs Undetectable Pathfinder", y);
            addInfo("Target: Geometry Dash 2.2081 / Geode SDK 5.11.2", y);
            addAction("Open Geode keybind settings", "OPEN", [] { openSettingsPopup(Mod::get()); }, y);
            addInfo("Menu key default: Z  |  Autoplay request: 4", y);
            addInfo("Settings apply immediately; reopen this menu to refresh values.", y);
            break;
    }
}

void PhosophursMenu::toggle() {
    if (g_menuInstance) {
        g_menuInstance->removeFromParentAndCleanup(true);
        return;
    }
    if (auto menu = PhosophursMenu::create()) menu->show();
}

void PhosophursMenu::onExit() {
    g_menuOpen = false;
    g_menuInstance = nullptr;
    Popup::onExit();
}
