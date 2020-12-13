#include "stdafx.h"
#include "PopupMenu.h"
#include "Constants.h"

PopupMenu::PopupMenu(std::vector<std::string> items) {
    this->items = items;
};

CRect PopupMenu::render(HDC hdc, CPoint pt, const std::vector<std::string>& selectedValues) {
    CSize popupSize = calculateSize(hdc);
    CRect menuRect = { pt.x, pt.y, pt.x + popupSize.cx, pt.y + popupSize.cy };
    FillRect(hdc, &menuRect, AMAN_BRUSH_TIMELINE_AHEAD);

    CRect rect = { pt.x, pt.y, pt.x + popupSize.cx, pt.y + popupSize.cy };
    rect.MoveToXY(pt);

    auto oldObject = SelectObject(hdc, AMAN_LEGEND_FONT);
    auto oldTextColor = SetTextColor(hdc, AMAN_COLOR_MENU_TEXT);
    auto oldBkColor = SetBkColor(hdc, AMAN_COLOR_MENU_TEXT_BACKGROUND_HOVER);

    for (int i = 0; i < items.size(); i++) {
        CRect previous = renderMenuItem(hdc, items[i].c_str(), rect, i, selectedValues);
        rect.MoveToY(previous.bottom);
    }

    SelectObject(hdc, oldObject);
    SetTextColor(hdc, oldTextColor);
    SetBkColor(hdc, oldBkColor);

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


void PopupMenu::update(const std::vector<std::string>& activeItems) {
    this->activeItems = activeItems;
}

const std::string& PopupMenu::getClickedItem(CPoint pt) {
    for (auto& item : interactiveAreas) {
        if(item.second.PtInRect(pt)) return items[item.first];
    }
}

bool PopupMenu::onMouseHover(CPoint pt) {
    for (auto& item : interactiveAreas) {
        if (item.second.PtInRect(pt)) {
            bool shouldRedraw = item.first != lastHoveredIndex;
            this->lastHoveredIndex = item.first;
            return shouldRedraw;
        }
    }
    lastHoveredIndex = -1;
    return false;
}

CRect PopupMenu::renderMenuItem(HDC hdc, const std::string& text, CRect area, int index, const std::vector<std::string>& selectedValues) {
    bool isActive = std::find(selectedValues.begin(), selectedValues.end(), text) != selectedValues.end();
    auto label = isActive ? "x " + text : "  " + text;

    int minWidth = area.Width();
    bool isHovered = index == lastHoveredIndex;
    auto oldBkMode = SetBkMode(hdc, isHovered ? OPAQUE : TRANSPARENT);
    DrawText(hdc, label.c_str(), label.length(), &area, DT_LEFT | DT_CALCRECT);
    area.right = area.left + minWidth;
    DrawText(hdc, label.c_str(), label.length(), &area, DT_LEFT);
    interactiveAreas[index] = { area };
    SetBkMode(hdc, oldBkMode);
    return area;
}