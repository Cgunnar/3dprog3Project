#include "pch.h"

#include "Window.h"
#include "Mouse.h"
#include "RenderingTypes.h"
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

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0;
	rid.hwndTarget = nullptr;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) assert(false); //failed to register device

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);

	Mouse::Init(m_hWnd);

	m_isStarting = false;
}

Window::~Window()
{
	Mouse::Destroy();
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
	if (renderer)
	{
		SetFullscreen(renderer->GetRenderingSettings().fullscreemState, false);
	}
}

std::pair<uint32_t, uint32_t> Window::GetWidthAndHeight()
{
	if (s_windowInstance)
	{
		RECT rect;
		GetClientRect(s_windowInstance->m_hWnd, &rect);
		uint32_t width = rect.right - rect.left;
		uint32_t height = rect.bottom - rect.top;
		if (s_windowInstance->m_renderer != nullptr && (s_windowInstance->m_renderer->m_width != width || s_windowInstance->m_renderer->m_height != height))
		{
			utl::PrintDebug("Window::GetWidthAndHeight() gives strange result, window might not be open");
		}
		return std::make_pair(width, height);
	}
	else
	{
		std::pair<uint32_t, uint32_t>();
	}
}

void Window::SetFullscreen(FullscreenState state, bool remeberSize, uint32_t width, uint32_t height)
{
	if (m_renderer == nullptr) return;

	switch (m_fullscreenState)
	{
	case FullscreenState::windowed:
	{
		switch (state)
		{
		case FullscreenState::borderLess:
			SetBorderLess(remeberSize);
			break;
		case FullscreenState::fullscreen:
			if(remeberSize)
				GetWindowRect(m_hWnd, &m_windowModeRect);
			bool fullscreen = m_renderer->SetFullscreen(true);
			assert(fullscreen);
			break;
		}

		break;
	}
	case FullscreenState::borderLess:
	{
		switch (state)
		{
		case FullscreenState::windowed:
			SetWindowed(width, height);
			break;
		case FullscreenState::fullscreen:
			bool fullscreen = m_renderer->SetFullscreen(true);
			assert(fullscreen);
			break;
		}

		break;
	}
	case FullscreenState::fullscreen:
	{
		switch (state)
		{
		case FullscreenState::windowed:
			SetWindowed(width, height);
			break;
		case FullscreenState::borderLess:
			SetBorderLess(remeberSize);
			break;
		}

		break;
	}
	}
	m_fullscreenState = state;
}

FullscreenState Window::GetFullscreenState() const
{
	return m_fullscreenState;
}

void Window::SetBorderLess(bool remeberSize)
{
	if (m_fullscreenState == FullscreenState::fullscreen)
	{
		bool fullscreen = m_renderer->SetFullscreen(false);
		assert(!fullscreen);
	}
	else if (remeberSize)
	{
		GetWindowRect(m_hWnd, &m_windowModeRect);
	}

	SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

	IDXGIOutput* output;
	HRESULT hr = m_renderer->m_swapchain->GetContainingOutput(&output);
	assert(SUCCEEDED(hr));
	DXGI_OUTPUT_DESC outputDesc;
	hr = output->GetDesc(&outputDesc);
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

void Window::SetWindowed(uint32_t width, uint32_t height)
{
	if(width == 0) width = abs(m_windowModeRect.right - m_windowModeRect.left);
	if (height == 0) height = abs(m_windowModeRect.bottom - m_windowModeRect.top);
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
	case WM_MOVE:
	{
		if (m_renderer)
		{
			auto oc = m_renderer->GetOutputCapabilities();
			if (oc.Monitor != m_renderer->m_outputDesc.Monitor)
			{
				utl::PrintDebug("Display changed");
				m_renderer->DisplayChanged();
			}
		}
	}
	case WM_SYSKEYDOWN:
	{
		// Handle ALT+ENTER:
		if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
		{
			if (m_fullscreenState == FullscreenState::windowed || m_fullscreenState == FullscreenState::borderLess)
				SetFullscreen(FullscreenState::fullscreen);
			else if (m_fullscreenState == FullscreenState::fullscreen)
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
			if(Mouse::s_mouseInstance) Mouse::Get().m_windowOutOfFocus = false;
		}
		else
		{
			//close
			m_fullscreenStateWhenOnOutOfFocus = m_fullscreenState;
			Mouse::Get().m_windowOutOfFocus = true;
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
	case WM_MOUSEMOVE:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;

		POINTS p = MAKEPOINTS(lParam);
		mouse.m_mouseState0.x = p.x;
		mouse.m_mouseState0.y = p.y;
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState0.z += GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		mouse.m_mouseState0.deltaZ += GET_WHEEL_DELTA_WPARAM(wParam) / 120;
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState0.LMBClicked = true;
		mouse.m_mouseState0.LMBHeld = true;
		return 0;
	}

	case WM_LBUTTONUP:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState0.LMBReleased = true;
		mouse.m_mouseState0.LMBHeld = false;
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState0.RMBClicked = true;
		mouse.m_mouseState0.RMBHeld = true;
		return 0;
	}

	case WM_RBUTTONUP:
	{
		Mouse& mouse = Mouse::Get();
		if (!m_isStarting && !m_isClosed && mouse.m_showCursor && ImGui::GetIO().WantCaptureMouse) return 0;
		mouse.m_mouseState0.RMBReleased = true;
		mouse.m_mouseState0.RMBHeld = false;
		return 0;
	}
	case WM_INPUT:
	{
		if (!m_isStarting && !m_isClosed && ImGui::GetIO().WantCaptureMouse) return 0;
		UINT bufferSize{};
		UINT errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &bufferSize, sizeof(RAWINPUTHEADER));
		assert(errorCode != -1);
		if (errorCode == -1) return 0;

		m_ridData.resize(bufferSize);
		errorCode = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, m_ridData.data(), &bufferSize, sizeof(RAWINPUTHEADER));
		assert(errorCode != -1);
		if (errorCode == -1) return 0;

		auto& myMouse = Mouse::Get();
		RAWINPUT& rawMouseInput = (RAWINPUT&)(*m_ridData.data());
		if (rawMouseInput.header.dwType == RIM_TYPEMOUSE)
		{
			if (rawMouseInput.data.mouse.lLastX || rawMouseInput.data.mouse.lLastY)
			{
				myMouse.m_mouseState0.deltaX += static_cast<float>(rawMouseInput.data.mouse.lLastX);
				myMouse.m_mouseState0.deltaY += static_cast<float>(rawMouseInput.data.mouse.lLastY);
			}
		}
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
