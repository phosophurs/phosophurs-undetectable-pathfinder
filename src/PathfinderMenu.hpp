#pragma once
#include <Geode/Geode.hpp>
#include <functional>
#include <string>
#include <vector>

class PhosophursMenu : public geode::Popup {
public:
    static PhosophursMenu* create();
protected:
    bool init() override;
    void onExit() override;
    void buildTabs();
    void buildPage();
    void selectTab(int tab);
    void addSection(char const* title, float& y);
    void addInfo(char const* text, float& y);
    void addAction(char const* label, char const* buttonText, std::function<void()> callback, float& y);
    void addToggle(char const* label, char const* setting, float& y);
    void addCycle(char const* label, char const* setting, float& y, std::vector<int> const& values, std::vector<std::string> const& names);
    void addFloatCycle(char const* label, char const* setting, float& y, std::vector<double> const& values);
    int m_tab = 0;
    cocos2d::CCNode* m_page = nullptr;
    cocos2d::CCMenu* m_tabMenu = nullptr;
};
