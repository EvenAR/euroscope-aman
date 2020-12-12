#pragma once

#include <string>

class Window {
public:
    Window(const std::string& className, const std::string& windowName);
    ~Window();

    void moveWindowBy(CPoint delta);
    void resizeWindowBy(CPoint delta);

protected:
    virtual void mousePressed(CPoint cursorPosition) {};
    virtual void mouseReleased(CPoint cursorPosition) {};
    virtual void mouseMoved(CPoint cursorPosition) {};
    virtual void mouseWheelSrolled(CPoint cursorPosition, short delta) {};
    virtual void windowClosed() {};
    virtual void drawContent(HWND hwnd) {};

    void requestRepaint();
    HWND hwnd;

private:
    const char* className;
    const char* windowName;

    DWORD threadId;

    static DWORD WINAPI lookForMessages(LPVOID lpParam);
    static LRESULT CALLBACK messageRouter(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT CALLBACK handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    bool create();
    void show(int nCmdShow);
    bool processNextMessage();


};

