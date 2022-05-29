#pragma once
#include "Renderer.h"
#include "CodeFromOlderProjects\Mouse.h"
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
	static std::pair<uint32_t, uint32_t> GetWidthAndHeight();
	void SetFullscreen(FullscreenState state, bool remeberSize = true, uint32_t width = 0, uint32_t height = 0);
	FullscreenState GetFullscreenState() const;
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
	Mouse::Mode m_mouseModeOnOutOfFocus = Mouse::Mode::Visible;
	std::vector<std::byte> m_ridData;
	void SetBorderLess(bool remeberSize);
	void SetWindowed(uint32_t width = 0, uint32_t height = 0);
	LRESULT CALLBACK HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

