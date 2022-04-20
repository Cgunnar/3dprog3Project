#pragma once
class Window
{
public:
	Window();
	~Window();

	bool Win32MsgPump();
private:
	static Window* s_windowInstance;
	const wchar_t* m_wndClassName{ L"wcName" };
	HINSTANCE m_hInst;
	HWND m_hWnd;

	bool m_firstActivate = false;
	bool m_isClosed = false;
	bool m_isStarting = true;
	bool m_wasFullscreenWhenOnOutOfFocus = false;

	std::vector<std::byte> m_ridData;

	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

