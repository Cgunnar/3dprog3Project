#pragma once
#include "Renderer.h"
class Window
{
public:
	Window();
	~Window();
	Window(const Window& other) = delete;
	Window& operator=(const Window& other) = delete;
	HWND GetHWND() const;
	void ChangeWindowText(std::string text);
	bool Win32MsgPump();

	void SetRenderer(Renderer* renderer);
private:
	static Window* s_windowInstance;
	const wchar_t* m_wndClassName{ L"wcName" };
	HINSTANCE m_hInst;
	HWND m_hWnd;
	Renderer* m_renderer = nullptr;
	bool m_firstActivate = false;
	bool m_isClosed = false;
	bool m_isStarting = true;
	bool m_wasFullscreenWhenOnOutOfFocus = false;

	std::vector<std::byte> m_ridData;

	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

