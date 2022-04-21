#pragma once
#include "Renderer.h"
class Window
{
public:
	Window();
	~Window();
	Window(const Window& other) = delete;
	Window& operator=(const Window& other) = delete;

	enum class FullscreenState
	{
		windowed,
		borderLess,
		fullscreen //this is not full-screen exclusive mode
	};

	HWND GetHWND() const;
	void ChangeWindowText(std::string text);
	bool Win32MsgPump();

	void SetRenderer(Renderer* renderer);
	
	void SetFullscreen(FullscreenState state);
private:
	
	FullscreenState m_fullscreenState = FullscreenState::windowed;
	static Window* s_windowInstance;
	const wchar_t* m_wndClassName{ L"wcName" };
	HINSTANCE m_hInst;
	HWND m_hWnd;
	RECT m_windowModeRect;
	Renderer* m_renderer = nullptr;
	bool m_isFullscreen = false;
	bool m_firstActivate = false;
	bool m_isClosed = false;
	bool m_isStarting = true;
	FullscreenState m_fullscreenStateWhenOnOutOfFocus = FullscreenState::windowed;

	std::vector<std::byte> m_ridData;
	void SetBorderLess();
	void SetWindowed();
	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

