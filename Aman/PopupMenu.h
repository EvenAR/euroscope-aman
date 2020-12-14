#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

class PopupMenu {
public:
    PopupMenu(
        const std::string& name, 
        std::vector<std::string> ids,
        std::function<void(const std::string& clickedItem)> onClick
    );
    CRect render(HDC hdc, CPoint pt);

    bool onMouseClick(CPoint pt);
    bool onMouseHover(CPoint pt);
    const std::string& getName() { return name; };
    void setActiveItems(const std::vector<std::string>& activeItems);
private:
    std::string name;
    std::vector<std::string> items;
    std::vector<std::string> activeItems;
    std::unordered_map<int, CRect> clickAreas;
    int lastHoveredIndex;
    std::function<void(const std::string& clickedItem)> onClick;

    CSize calculateSize(HDC hdc);
    CRect renderMenuItem(HDC hdc, const std::string& text, CRect area, int index);
};