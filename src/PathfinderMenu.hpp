#pragma once

#include <Geode/Geode.hpp>
#include <functional>
#include <string>
#include <vector>

class PhosophursMenu : public geode::Popup {
public:
    static PhosophursMenu* create();

protected:
    bool init();
    void buildTabs();
    void buildPage();

    void selectTab(int tab);
    void addSection(char const* title, float& y);
    void addToggle(char const* label, char const* setting, float& y);
    void addCycle(char const* label, char const* setting, float& y,
                  std::vector<int> const& values, std::vector<std::string> const& names);
    void addFloatCycle(char const* label, char const* setting, float& y,
                       std::vector<double> const& values);
    void addAction(char const* label, std::function<void()> callback, float& y);
    void addInfo(char const* label, float& y);

    int m_tab = 0;
    cocos2d::CCNode* m_page = nullptr;
    cocos2d::CCMenu* m_tabMenu = nullptr;
};
