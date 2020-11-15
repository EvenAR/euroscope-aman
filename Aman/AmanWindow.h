#pragma once
#include <vector>

#define AIRCRAFT_DATA (WM_APP + 101)
#define CLOSE_WINDOW (WM_APP + 101)
#define AMAN_WINDOW_CLASS_NAME "AmanWindow"
#define AMAN_WINDOW_TITLE "AMAN"

class AmanController;
class AmanTimeline;
class AmanRenderer;
class TitleBar;

class AmanWindow {

public:
    AmanWindow(AmanController* controller, TitleBar* titleBar);
    ~AmanWindow();

    void update(const std::vector<AmanTimeline*>& timelines);
    void collapse();
    void expand();
    bool isExpanded();
    void moveWindowBy(CPoint delta);
    void resizeWindowBy(CPoint delta);

    AmanTimeline* getTimelineAt(const std::vector<AmanTimeline*>& timelines, CPoint cursorPosition);

private:
    TitleBar* titleBar;
    DWORD threadId;

    static void drawContent(HWND hwnd);
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI threadProc(LPVOID lpParam);
};
