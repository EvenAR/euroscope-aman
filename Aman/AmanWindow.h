#pragma once
#include <vector>
#include <mutex>

#include "Window.h"

class AmanController;
class AmanTimeline;
class AmanRenderer;
class TitleBar;

class AmanWindow : public Window {

public:
    AmanWindow(AmanController* controller, TitleBar* titleBar);
    ~AmanWindow();

    void update(const std::vector<AmanTimeline*>& timelines);
    void collapse();
    void expand();
    bool isExpanded();

    AmanTimeline* getTimelineAt(const std::vector<AmanTimeline*>& timelines, CPoint cursorPosition);

private:
    AmanController* controller;
    TitleBar* titleBar;

    std::vector<AmanTimeline*> currentTimelines;
    std::mutex renderTimelinesMutex;

    int originalHeight;

    void mousePressed(CPoint cursorPosition) override;
    void mouseReleased(CPoint cursorPosition) override;
    void mouseMoved(CPoint cursorPosition) override;
    void mouseWheelSrolled(CPoint cursorPosition, short delta) override;
    void windowClosed() override;
    void drawContent(HDC hdc, CRect windowRect) override;
};
