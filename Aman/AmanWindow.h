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

class AmanWindow : public Window {

public:
    AmanWindow(AmanController* controller, std::shared_ptr<TitleBar> titleBar, std::set<std::string> ids);
    ~AmanWindow();

    void update(std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> timelines);
    void collapse();
    void expand();
    bool isExpanded();

    std::shared_ptr<AmanTimeline> getTimelineAt(std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> all, CPoint cursorPosition);

private:
    AmanController* controller;
    std::shared_ptr<TitleBar> titleBar;
    std::shared_ptr<MenuBar> menuBar;
    std::shared_ptr<PopupMenu> profilesMenu;

    std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> timelinesToRender;
    std::mutex renderTimelinesMutex;

    int originalHeight;
    CRect timelineView;

    void mousePressed(CPoint cursorPosClient) override;
    void mouseReleased(CPoint cursorPosClient) override;
    void mouseMoved(CPoint cursorPosClient) override;
    void mouseWheelSrolled(CPoint cursorPosClient, short delta) override;
    void windowClosed() override;
    void drawContent(HDC hdc, CRect clientRect) override;
};
