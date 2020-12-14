#include "stdafx.h"

#include <string>

#include "AmanController.h"
#include "Constants.h"
#include "TitleBar.h"

TitleBar::TitleBar() {}

CRect TitleBar::render(CRect clientRect, HDC memdc) {
    int buttonMargin = 5;
    int buttonSize = 10;

    this->titleBarRect = { 0, 0, clientRect.right, AMAN_TITLEBAR_HEIGHT };

    this->resizeButton.rect = { clientRect.Width() - buttonMargin - buttonSize, buttonMargin,
                               clientRect.Width() - buttonMargin, buttonMargin + buttonSize };

    this->closeButton.rect = { clientRect.Width() - buttonMargin * 2 - buttonSize * 2, buttonMargin,
                              clientRect.Width() - buttonSize - buttonMargin * 2, buttonMargin + buttonSize };

    // Start drawing
    FillRect(memdc, &this->titleBarRect, AMAN_BRUSH_TITLE_BAR_BACKGROUND);

    HBRUSH oldBrush = (HBRUSH)SelectObject(memdc, GetStockObject(NULL_BRUSH));
    HPEN oldPen = (HPEN)SelectObject(memdc, AMAN_WHITE_PEN);
    int oldBackground = SetBkMode(memdc, TRANSPARENT);
    int oldTextColor = SetTextColor(memdc, AMAN_COLOR_TITLE_BAR_TEXT);

    DrawText(memdc, this->title, strlen(this->title), &this->titleBarRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Resize button
    CRect btnRect = this->resizeButton.rect;
    Rectangle(memdc, btnRect.left, btnRect.top, btnRect.left + 5, btnRect.top + 5);
    Rectangle(memdc, btnRect.left, btnRect.top, btnRect.left + 8, btnRect.top + 8);
    Rectangle(memdc, btnRect.left, btnRect.top, btnRect.left + 11, btnRect.top + 11);

    btnRect = this->closeButton.rect;
    Rectangle(memdc, btnRect.left, btnRect.top, btnRect.left + 11, btnRect.top + 11);

    SelectObject(memdc, AMAN_BRUSH_MENU_ICON_FILL);
    Ellipse(memdc, btnRect.left + 3, btnRect.top + 3, btnRect.left + 8, btnRect.top + 8);

    SelectObject(memdc, oldBrush);
    SelectObject(memdc, oldPen);
    SetBkMode(memdc, oldBackground);

    return this->titleBarRect;
}

void TitleBar::mousePressed(CPoint cursorPosition) {
    if (this->resizeButton.rect.PtInRect(cursorPosition)) {
        this->emit("RESIZE_PRESSED");
    } else if (this->closeButton.rect.PtInRect(cursorPosition)) {
        this->emit("COLLAPSE_CLICKED");
    } else if (this->titleBarRect.PtInRect(cursorPosition)) {
        this->emit("MOUSE_PRESSED");
    }
}

TitleBar::~TitleBar() {}
