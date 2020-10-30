#pragma once

#include <vector>

class AmanWindow;
class AmanPlugIn;
class AmanTimeline;

class AmanController
{
	
private:
	bool expanded = true;
	bool moveWindow = false;
	bool doResize = false;
	CPoint mouseDownPosition;
	CPoint previousMousePosition;

	AmanPlugIn* amanPlugin;	// Model
	AmanWindow* amanWindow;	// View1

public:
	AmanController(AmanPlugIn* plugin);
	void openWindow();
	void windowClosed();
	void mousePressed(CRect windowRect, CPoint cursorPosition);
	void mouseReleased(CRect windowRect, CPoint cursorPosition);
	void mouseMoved(CRect windowRect, CPoint cursorPosition);
	void mouseWheelSrolled(CPoint cursorPosition, short delta);
	void timelinesUpdated();

	void resizebuttonPressed();
	void titleBarPressed();
	void collapseButtonPressed();
	std::vector<AmanTimeline*>* getTimelines();

	~AmanController();
};

