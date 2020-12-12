#pragma once
#include <vector>
#include <mutex>

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
    bool handleMessages();
    bool create();
    void show(int nCmdShow);

    AmanTimeline* getTimelineAt(const std::vector<AmanTimeline*>& timelines, CPoint cursorPosition);

private:
    AmanController* gpController;
    TitleBar* gpTitleBar;

    int originalHeight;
    std::vector<AmanTimeline*> gpCurrentTimelines;
    std::mutex renderTimelinesMutex;
    DWORD threadId;
    HWND hwnd;

    static DWORD WINAPI lookForMessages(LPVOID lpParam);
    static LRESULT CALLBACK messageRouter(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT CALLBACK handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void drawContent(HWND hwnd);
};
