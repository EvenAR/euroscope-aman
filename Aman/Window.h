#pragma once

#include <string>

class Window {
public:
    Window(const std::string& className, const std::string& windowName);
    ~Window();

    void moveWindowBy(CPoint delta);
    void resizeWindowBy(CPoint delta);

protected:
    HWND hwnd;
    void requestRepaint();

    // Should be overridden by child class:
    virtual void mousePressed(CPoint cursorPosClient) {};
    virtual void mouseReleased(CPoint cursorPosClient) {};
    virtual void mouseMoved(CPoint cursorPosClient) {};
    virtual void mouseWheelSrolled(CPoint cursorPosClient, short delta) {};
    virtual void windowClosed() {};
    virtual void drawContent(HDC hdc, CRect clientRect) {};

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
    void render(HWND hwnd);
};

