#pragma once

#include <vector>
#include <string>

class TitleBar;
class AmanWindow;
class AmanPlugIn;
class AmanTimeline;

class AmanController {
public:
    AmanController(AmanPlugIn* plugin);
    void openWindow();
    void windowClosed();
    void mousePressed(CPoint cursorPosition);
    void mouseReleased(CPoint cursorPosition);
    void mouseMoved(CPoint cursorPosition);
    void mouseWheelSrolled(CPoint cursorPosition, short delta);
    void dataUpdated();
    void toggleTimeline(const std::string& id);

    ~AmanController();

private:
    bool moveWindow = false;
    bool doResize = false;
    CPoint mouseDownPosition;
    CPoint previousMousePosition;

    AmanPlugIn* amanPlugin; // Model
    AmanWindow* amanWindow; // View1
    TitleBar* titleBar;
    std::vector<std::string> activeTimelines;
};
