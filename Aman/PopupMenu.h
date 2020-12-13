#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class PopupMenu {
public:
    PopupMenu(std::vector<std::string> ids);
    CRect render(HDC hdc, CPoint pt, const std::vector<std::string>& selectedValues);
    void update(const std::vector<std::string>& activeItems);

    const std::string& getClickedItem(CPoint pt);
    bool onMouseHover(CPoint pt);

private:
    std::vector<std::string> items;
    std::vector<std::string> activeItems;
    std::unordered_map<int, CRect> interactiveAreas;
    int lastHoveredIndex;

    CSize calculateSize(HDC hdc);
    CRect renderMenuItem(HDC hdc, const std::string& text, CRect area, int index, const std::vector<std::string>& selectedValues);
};