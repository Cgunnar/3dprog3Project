#include "pch.h"

#include "Window.h"

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
}

Window::~Window()
{
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



//extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT Window::HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/*if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	{
		return true;
	}*/

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
		return 0;
	}
	case WM_ACTIVATEAPP:
	{
		//alt + tab
		if (wParam)
		{
			//open
		}
		else
		{
			//close
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
