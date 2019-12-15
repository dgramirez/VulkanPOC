#ifdef _WIN32
#include "myWindow.h"
#include <Windows.h>

HWND hWnd = nullptr;
HINSTANCE hInst = nullptr;
bool runProgram = true;
unsigned char wndEventMask;
LRESULT (*WndProcPtr)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LPCWSTR lpClassName = L"dgramirezVulkan";
LPCWSTR lpWindowName = L"Title";

ATOM MyRegisterClass();
void ProcessMessage();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

myWindow::myWindow(void **_outHInst, void **_outHWnd, const char **_outAddMsg) {
	WndProcPtr = WndProc;
	if (_outHInst || _outHWnd)
		init(_outHInst, _outHWnd, _outAddMsg);
}

bool myWindow::init(void **_outHInst, void **_outHWnd, const char **_outAddMsg) {
	//Initialize HINSTANCE
	MyRegisterClass();

	//Perform Initialization
	hWnd = CreateWindowW(lpClassName, lpWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, nullptr, nullptr, hInst, nullptr);
	if (!hWnd) {
		//RIP: Failed to create window. return error and update
		if (_outAddMsg)
			*_outAddMsg = "Failed to Create Window!";
		if (_outHInst)
			*_outHInst = nullptr;
		return true;
	}

	//Show and Update
	ShowWindow(hWnd, 1);
	UpdateWindow(hWnd);

	//SUCCESS: return no errors!
	if (_outAddMsg)
		*_outAddMsg = "Window was created successfully!";
	if (_outHInst)
		*_outHInst = hInst;
	if (_outHWnd)
		*_outHWnd = hWnd;
	return false;
}

void myWindow::getInstance(void **_outInstance)
{
	if (_outInstance)
		*_outInstance = hInst;
}

void myWindow::getWindow(void **_outWindow)
{
	if (_outWindow)
		*_outWindow = hWnd;
}

bool myWindow::isRunning() const
{
	return runProgram;
}

void myWindow::getEvents(unsigned char &_outEvents)
{
	ProcessMessage();
	_outEvents = wndEventMask;
}

void myWindow::getEvents(WndEvent &_outEvents)
{
	ProcessMessage();
	_outEvents = static_cast<WndEvent>(wndEventMask);
}

void myWindow::toggleEvent(const unsigned char& _mask)
{
	wndEventMask ^= _mask;
}

void myWindow::toggleEvent(const WndEvent& _mask)
{
	wndEventMask ^= static_cast<unsigned char>(_mask);
}

void myWindow::setWinProc(void *_wndProc)
{
	WndProcPtr = static_cast<LRESULT (*)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)>(_wndProc);
}

void myWindow::getWinProc(void** _outWndProc)
{
	*_outWndProc = static_cast<void*>(WndProcPtr);
}

void myWindow::getRect(int& width, int& height)
{
	RECT wnd;
	GetWindowRect(hWnd, &wnd);

	width = wnd.right - wnd.left;
	height = wnd.bottom - wnd.top;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOVE:
		wndEventMask |= static_cast<unsigned char>(myWindow::WndEvent::MOVE);
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			wndEventMask |= static_cast<unsigned char>(myWindow::WndEvent::MINIMIZE);
		else if (wParam == SIZE_MAXIMIZED)
			wndEventMask |= static_cast<unsigned char>(myWindow::WndEvent::MAXIMIZE);
		else if (wParam == SIZE_RESTORED)
			wndEventMask |= static_cast<unsigned char>(myWindow::WndEvent::RESIZE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		runProgram = false;
		break;
	}
	return 0;
}

ATOM MyRegisterClass()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ImGui_ImplWin32_WndProcHandler;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = lpClassName;
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

void ProcessMessage()
{
	MSG msg;
	GetMessage(&msg, nullptr, 0, 0);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

#endif