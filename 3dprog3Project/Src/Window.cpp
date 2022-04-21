#include "pch.h"

#include "Window.h"
#include <imgui_impl_win32.h>

Window* Window::s_windowInstance = nullptr;

Window::Window()
{
	assert(!s_windowInstance);
	s_windowInstance = this;

	m_hInst = GetModuleHandle(nullptr);

	WNDCLASSEX wc{};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = m_hInst;
	wc.lpszClassName = m_wndClassName;
	wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {assert(s_windowInstance); return s_windowInstance->HandleMsg(hwnd, uMsg, wParam, lParam); };
	//wc.lpfnWndProc = HandleMsg;
	RegisterClassEx(&wc);

	RECT wr;
	wr.left = 100;
	wr.right = 1280 + wr.left;
	wr.top = 100;
	wr.bottom = 720 + wr.top;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	m_hWnd = CreateWindowEx(0, m_wndClassName, L"WindowText",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, m_hInst, nullptr); //last pointer is to some optional data of any kind that kan be usefull when getting messages

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	GetWindowRect(m_hWnd, &m_windowModeRect);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);

}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	s_windowInstance = nullptr;
}

HWND Window::GetHWND() const
{
	return m_hWnd;
}

void Window::ChangeWindowText(std::string text)
{
	std::wstringstream win32text;
	win32text << text.c_str();
	std::wstring ws = win32text.str();
	SetWindowText(m_hWnd, ws.c_str());
}

bool Window::Win32MsgPump()
{
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
		{
			return false;
		}
	}
	return true;
}

void Window::SetRenderer(Renderer* renderer)
{
	m_renderer = renderer;
}

void Window::SetFullscreen(FullscreenState state)
{
	if (m_renderer == nullptr) return;
	
	switch (m_fullscreenState)
	{
	case Window::FullscreenState::windowed:
	{
		switch (state)
		{
		case Window::FullscreenState::borderLess:
			SetBorderLess();
			break;
		case Window::FullscreenState::fullscreen:
			bool fullscreen = m_renderer->SetFullscreen(true);
			assert(fullscreen);
			break;
		}

		break;
	}
	case Window::FullscreenState::borderLess:
	{
		switch (state)
		{
		case Window::FullscreenState::windowed:
			SetWindowed();
			break;
		case Window::FullscreenState::fullscreen:
			bool fullscreen = m_renderer->SetFullscreen(true);
			assert(fullscreen);
			break;
		}

		break;
	}
	case Window::FullscreenState::fullscreen:
	{
		switch (state)
		{
		case Window::FullscreenState::windowed:
			SetWindowed();
			break;
		case Window::FullscreenState::borderLess:
			SetBorderLess();
			break;
		}

		break;
	}
	}
	m_fullscreenState = state;
}

void Window::SetBorderLess()
{
	if (m_fullscreenState == FullscreenState::fullscreen)
	{
		bool fullscreen = m_renderer->SetFullscreen(false);
		assert(!fullscreen);
	}
	else
	{
		GetWindowRect(m_hWnd, &m_windowModeRect);
	}

	SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

	IDXGIOutput* output;
	HRESULT hr = m_renderer->m_swapchain->GetContainingOutput(&output);
	if (FAILED(hr))
	{
		utl::PrintDebug("Window::SetFullscreen(bool fullscreen) failed");
		//likely laptop
		assert(false);
	}
	else
	{
		DXGI_OUTPUT_DESC outputDesc;
		HRESULT hr = output->GetDesc(&outputDesc);
		assert(SUCCEEDED(hr));
		output->Release();
		RECT rect = outputDesc.DesktopCoordinates;


		SetWindowPos(
			m_hWnd,
			HWND_TOPMOST,
			rect.left,
			rect.top,
			rect.right,
			rect.bottom,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);


		ShowWindow(m_hWnd, SW_MAXIMIZE);
		auto mode = m_renderer->GetBestDisplayMode();
		m_renderer->OnResize(mode.Width, mode.Height, true);
		m_fullscreenState = FullscreenState::borderLess;
	}
}

void Window::SetWindowed()
{
	int width = abs(m_windowModeRect.right - m_windowModeRect.left);
	int height = abs(m_windowModeRect.bottom - m_windowModeRect.top);
	if (m_fullscreenState == FullscreenState::fullscreen)
	{
		bool state = m_renderer->SetFullscreen(false, width, height);
		assert(state == false);
	}

	SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
	SetWindowPos(
		m_hWnd,
		HWND_NOTOPMOST,
		m_windowModeRect.left,
		m_windowModeRect.top,
		width,
		height,
		SWP_FRAMECHANGED | SWP_NOACTIVATE);

	ShowWindow(m_hWnd, SW_NORMAL);

	m_fullscreenState = FullscreenState::windowed;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Window::HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch (uMsg)
	{
	case WM_CLOSE:
	{
		m_isClosed = true;
		std::cout << "WM_CLOSE" << std::endl;
		DestroyWindow(hwnd);
		return 0;
	}
	case WM_DESTROY:
	{
		std::cout << "WM_DESTROY" << std::endl;
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE:
	{
		auto width = LOWORD(lParam);
		auto height = HIWORD(lParam);
		std::cout << "WM_SIZE\twidth: " << width << " height: " << height << std::endl;

		if (m_renderer)
		{
			m_renderer->OnResize(width, height, false);
		}

		return 0;
	}

	case WM_SYSKEYDOWN:
	{
		// Handle ALT+ENTER:
		if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
		{
			if(m_fullscreenState == Window::FullscreenState::windowed || m_fullscreenState == Window::FullscreenState::borderLess)
				SetFullscreen(FullscreenState::fullscreen);
			else if(m_fullscreenState == Window::FullscreenState::fullscreen)
				SetFullscreen(FullscreenState::windowed);
			return 0;
		}
		break;
	}
	case WM_ACTIVATEAPP:
	{
		//alt + tab
		if (wParam)
		{
			//open
			SetFullscreen(m_fullscreenStateWhenOnOutOfFocus);
		}
		else
		{
			//close
			m_fullscreenStateWhenOnOutOfFocus = m_fullscreenState;
			SetFullscreen(FullscreenState::windowed);
		}
		return 0;
	}
	case WM_ACTIVATE:
	{
		if (!m_firstActivate)
		{
			m_firstActivate = true;
			break;
		}
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
