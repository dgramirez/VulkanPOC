#ifndef MYWINDOW_H
#define MYWINDOW_H

class myWindow {
public:
	enum class WndEvent { NOTHING = 0, MOVE = 0x1, RESIZE = 0x2, MINIMIZE = 0x4, MAXIMIZE = 0x8 };

	myWindow(void** _outHInst = nullptr, void** _outHWnd = nullptr, const char** _outAddMsg = nullptr);
	bool init(void** _outHInst = nullptr, void** _outHWnd = nullptr, const char** _outAddMsg = nullptr);

	void getInstance(void** _outInstance);
	void getWindow(void** _outWindow);

	bool isRunning() const;
	__declspec(property(get = isRunning)) bool run;

	void getEvents(unsigned char& _outEvents);
	void getEvents(WndEvent& _outEvents);

	void toggleEvent(const unsigned char& _mask);
	void toggleEvent(const WndEvent& _mask);

	void setWinProc(void *_wndProc);
	void getWinProc(void **_outWndProc);

	void getRect(int &width, int& height);
private:
};

#endif