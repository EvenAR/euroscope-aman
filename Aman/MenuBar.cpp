#include "stdafx.h"
#include "MenuBar.h"
#include "PopupMenu.h"
#include "Constants.h"

MenuBar::MenuBar() {
}

void MenuBar::addPopupMenu(std::shared_ptr<PopupMenu> popupMenu) {
    menus.push_back({false, CRect(), popupMenu });
}

void MenuBar::render(HDC hdc, CRect rect) {
    FillRect(hdc, &rect, AMAN_BRUSH_MENU_BACKGROUND);
    int buttonOffsetX = 0;

    auto oldBkMode = SetBkMode(hdc, TRANSPARENT);
    auto oldFont = SelectObject(hdc, AMAN_MENU_FONT);
    auto oldTextColor = SetTextColor(hdc, AMAN_COLOR_MENU_TEXT);
    auto oldBkColor = SetBkColor(hdc, AMAN_COLOR_MENU_TEXT_BACKGROUND_HOVER);

    for (auto& menu : menus) {
        rect.MoveToX(buttonOffsetX);
        menu.openBtnArea = renderButton(hdc, menu.popup->getName(), rect, OPEN_BUTTON, menu.isOpen);
        if (menu.isOpen) {
            menu.popup->render(hdc, { menu.openBtnArea.left, menu.openBtnArea.bottom });
        }
        buttonOffsetX = menu.openBtnArea.right + 10;
    }

    SelectObject(hdc, oldFont);
    SetTextColor(hdc, oldTextColor);
    SetBkColor(hdc, oldBkColor);
    SetBkMode(hdc, oldBkMode);
}

CRect MenuBar::renderButton(HDC hdc, const std::string& text, CRect area, AREA_ID id, bool isActive) {
    // Renders the button and saves its bounding rectangle for later click detection
    auto oldBkMode = SetBkMode(hdc, isActive ? OPAQUE : TRANSPARENT);
    DrawText(hdc, text.c_str(), text.length(), &area, DT_LEFT | DT_CALCRECT);
    DrawText(hdc, text.c_str(), text.length(), &area, DT_LEFT);
    return area;
    SetBkMode(hdc, oldBkMode);
}

// Returns true if parent window should re-render
bool MenuBar::onMouseMove(CPoint cursorPos) {
    for (auto& menu : menus) {
        if (menu.isOpen && menu.popup->onMouseHover(cursorPos)) {
            return true;
        }
    }
}

// Returns true if parent window should re-render
bool MenuBar::onMouseClick(CPoint cursorPos) {
    for (auto& menu : menus) {
        if (menu.isOpen) {
            if (menu.popup->onMouseClick(cursorPos)) {
                return true;
            } else {
                menu.isOpen = !menu.isOpen;
                return true;
            }
        } else if (menu.openBtnArea.PtInRect(cursorPos)) {
            menu.isOpen = !menu.isOpen;
        }
    }
    return false;
}

bool MenuBar::onMouseScroll(short delta) {
    bool oneClosed = false;
    for (auto& menu : menus) {
        if (menu.isOpen) {
            menu.isOpen = !menu.isOpen;
            oneClosed = true;
        } 
    }
    return oneClosed;
}
