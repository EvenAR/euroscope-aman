#include "stdafx.h"
#include "PopupMenu.h"
#include "Constants.h"

PopupMenu::PopupMenu(
    const std::string& name,
    std::vector<std::string> options,
    std::function<void(const std::string& clickedItem)> onClick
) {
    this->name = name;
    this->items = options;
    this->onClick = onClick;
};

CRect PopupMenu::render(HDC hdc, CPoint pt) {
    CSize popupSize = calculateSize(hdc);
    CRect menuRect = { pt.x, pt.y, pt.x + popupSize.cx, pt.y + popupSize.cy };
    FillRect(hdc, &menuRect, AMAN_BRUSH_MENU_BACKGROUND);

    CRect rect = { pt.x, pt.y, pt.x + popupSize.cx, pt.y + popupSize.cy };
    rect.MoveToXY(pt);

    for (int i = 0; i < items.size(); i++) {
        CRect previous = renderMenuItem(hdc, items[i].c_str(), rect, i);
        rect.MoveToY(previous.bottom);
    }

    return menuRect;
}

CSize PopupMenu::calculateSize(HDC hdc) {
    CRect estimatedArea;
    CPoint nextItemPos;
    int maxWidth = 0;
    for (auto& item : items) {
        estimatedArea.MoveToY(nextItemPos.y);
        std::string label = "  " + item;
        DrawText(hdc, label.c_str(), label.length(), &estimatedArea, DT_LEFT | DT_CALCRECT);
        nextItemPos.y = estimatedArea.bottom;
        maxWidth = estimatedArea.Width() > maxWidth ? estimatedArea.Width() : maxWidth;
    }

    return { maxWidth, nextItemPos.y };
};

bool PopupMenu::onMouseClick(CPoint pt) {
    for (auto& area : clickAreas) {
        if(area.second.PtInRect(pt)) {
            onClick(items[area.first]);
            return true;
        }
    }
}

bool PopupMenu::onMouseHover(CPoint pt) {
    for (auto& item : clickAreas) {
        if (item.second.PtInRect(pt)) {
            bool shouldRedraw = item.first != lastHoveredIndex;
            this->lastHoveredIndex = item.first;
            return shouldRedraw;
        }
    }
    lastHoveredIndex = -1;
    return false;
}

CRect PopupMenu::renderMenuItem(HDC hdc, const std::string& text, CRect area, int index) {
    bool isActive = std::find(activeItems.begin(), activeItems.end(), text) != activeItems.end();
    auto label = (isActive ? "x " : "  ") + text;

    int minWidth = area.Width();
    bool isHovered = index == lastHoveredIndex;

    auto oldBkMode = SetBkMode(hdc, isHovered ? OPAQUE : TRANSPARENT);

    DrawText(hdc, label.c_str(), label.length(), &area, DT_LEFT | DT_CALCRECT);
    area.right = area.left + minWidth;
    DrawText(hdc, label.c_str(), label.length(), &area, DT_LEFT);
    clickAreas[index] = { area };

    SetBkMode(hdc, oldBkMode);

    return area;
}

void PopupMenu::setActiveItems(const std::vector<std::string>& activeItems) {
    this->activeItems = activeItems;
}

void PopupMenu::setMenuItems(std::vector<std::string> items) {
    this->items = items;
}
