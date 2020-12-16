#pragma once
#include <vector>
#include <mutex>
#include <set>
#include <unordered_map>

#include "Constants.h"
#include "Window.h"

class AmanController;
class AmanTimeline;
class AmanRenderer;
class TitleBar;
class PopupMenu;
class MenuBar;

typedef std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> timelineCollection;

class AmanWindow : public Window {

public:
    AmanWindow(AmanController* controller, std::set<std::string> availProfiles);
    ~AmanWindow();

    void update(timelineCollection timelines);

private:
    AmanController* controller;
    std::shared_ptr<TitleBar> titleBar;
    std::shared_ptr<MenuBar> menuBar;
    std::shared_ptr<PopupMenu> profilesMenu;

    timelineCollection timelinesToRender;
    std::mutex renderTimelinesMutex;

    int originalHeight;
    CRect timelineView;

    CRect originalSize;
    CPoint mouseDownPosition;
    CPoint prevMousePosition;
    bool moveWindow = false;
    bool doResize = false;

    void collapse();
    void expand();
    bool isExpanded();
    std::shared_ptr<AmanTimeline> getTimelineAt(timelineCollection all, CPoint cursorPosition);

    void mousePressed(CPoint cursorPosClient) override;
    void mouseReleased(CPoint cursorPosClient) override;
    void mouseMoved(CPoint cursorPosClient) override;
    void mouseWheelSrolled(CPoint cursorPosClient, short delta) override;
    void windowClosed() override;
    void drawContent(HDC hdc, CRect clientRect) override;
};
