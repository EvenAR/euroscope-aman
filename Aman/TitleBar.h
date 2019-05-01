#pragma once

class AmanController;

class TitleBar
{
private:
	struct Button {
		CRect rect;
		bool hovered;
	};
	
	const char* title = " AMAN - Arrival Manager";
	AmanController* controller;
	CRect titleBarRect;

	Button closeButton;
	Button resizeButton;

public:
	TitleBar(AmanController* controller);
	void render(CRect clientRect, HDC memdc);
	void mouseHover(CRect clientRect, CPoint cursorPosition);
	void mousePressed(CRect clientRect, CPoint cursorPosition);

	~TitleBar();
};

