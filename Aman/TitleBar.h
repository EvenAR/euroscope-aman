#pragma once

#include <functional>
#include <unordered_map>

#include "EventEmitter.h"

class AmanController;

class TitleBar : public EventEmitter {
public:
    TitleBar();
    CRect render(CRect clientRect, HDC memdc);
    void mousePressed(CPoint cursorPosition);

    ~TitleBar();

private:
    struct Button {
        CRect rect;
        bool hovered;
    };

    const char* title = " AMAN - Arrival Manager";
    CRect titleBarRect;

    Button closeButton;
    Button resizeButton;
};
