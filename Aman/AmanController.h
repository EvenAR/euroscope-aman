#pragma once

#include <vector>
#include <string>
#include <memory>

class TitleBar;
class AmanWindow;
class AmanPlugIn;
class AmanTimeline;

class AmanController {
public:
    AmanController(AmanPlugIn* model);
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

    AmanPlugIn* amanModel;
    std::shared_ptr<AmanWindow> amanWindow;
    std::shared_ptr<TitleBar> titleBar;

    std::vector<std::string> activeTimelines;
};
