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
    AmanWindow(AmanController* controller);
    void setAvailableTimelines(std::set<std::string> availProfiles);
    ~AmanWindow();

    void update(timelineCollection timelines);

    void setTimelineHorizon(const std::string& id, uint32_t minutes);

private:
    AmanController* controller;
    std::shared_ptr<TitleBar> titleBar;
    std::shared_ptr<MenuBar> menuBar;
    std::shared_ptr<PopupMenu> popupMenu;

    timelineCollection timelinesToRender;
    std::mutex renderTimelinesMutex;

    int originalHeight;
    CRect timelineView;

    CRect originalSize;
    CPoint mouseDownPosition;
    CPoint prevMousePosition;
    bool moveWindow = false;
    bool doResize = false;

    std::unordered_map<std::string, uint32_t> zoomLevels;

    void collapse();
    void expand();
    bool isExpanded();
    std::shared_ptr<AmanTimeline> getTimelineAt(timelineCollection all, CPoint cursorPosition);

    void mousePressed(CPoint cursorPosClient) override;
    void mouseReleased(CPoint cursorPosClient) override;
    void mouseMoved(CPoint cursorPosClient, CPoint cursorPosScreen) override;
    void mouseWheelSrolled(CPoint cursorPosClient, short delta) override;
    void closeRequested() override;
    void windowClosed() override;
    void drawContent(HDC hdc, CRect clientRect) override;
    uint32_t getZoomLevel(const std::string& id);
};
