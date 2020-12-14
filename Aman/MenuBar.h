#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

class PopupMenu;

class MenuBar {
public:
    MenuBar();

    void addPopupMenu(std::shared_ptr<PopupMenu> popupMenu);
    void render(HDC hdc, CRect rect);
    bool onMouseMove(CPoint cursorPos);
    bool onMouseClick(CPoint cursorPos);
    bool onMouseScroll(short delta);
private:
    enum AREA_ID {
        OPEN_BUTTON,
        PROFILES_MENU
    };

    struct PopupState {
        bool isOpen;
        CRect openBtnArea;
        std::shared_ptr<PopupMenu> popup;
    };

    CRect renderButton(HDC hdc, const std::string& text, CRect area, AREA_ID id, bool isActive);
    std::vector<PopupState> menus;
};

