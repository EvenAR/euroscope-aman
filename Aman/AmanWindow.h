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


struct ClickableElement {
    CRect area;
};

class AmanWindow : public Window {

public:
    AmanWindow(AmanController* controller, TitleBar* titleBar, std::set<std::string> ids);
    ~AmanWindow();

    void update(std::shared_ptr<std::vector<AmanTimeline*>> timelines);
    void collapse();
    void expand();
    bool isExpanded();

    AmanTimeline* getTimelineAt(std::shared_ptr<std::vector<AmanTimeline*>> timelines, CPoint cursorPosition);

private:
    enum AREA_ID {
        OPEN_BUTTON,
        PROFILES_MENU
    };

    AmanController* controller;
    TitleBar* titleBar;
    PopupMenu* popupMenu;

    std::shared_ptr<std::vector<AmanTimeline*>> timelinesToRender;
    std::mutex renderTimelinesMutex;

    int originalHeight;

    void mousePressed(CPoint cursorPosition) override;
    void mouseReleased(CPoint cursorPosition) override;
    void mouseMoved(CPoint cursorPosition) override;
    void mouseWheelSrolled(CPoint cursorPosition, short delta) override;
    void windowClosed() override;
    void drawContent(HDC hdc, CRect windowRect) override;

    CRect renderButton(HDC hdc, const std::string& text, CRect area, AREA_ID id);

    std::unordered_map<AREA_ID, ClickableElement> interactiveAreas;

    int findButtonAt(CPoint point);

    bool showOpenMenu;
};
