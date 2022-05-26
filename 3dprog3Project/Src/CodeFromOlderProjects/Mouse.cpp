#include "pch.h"
#include "Mouse.h"

Mouse* Mouse::s_mouseInstance = nullptr;

void Mouse::Init(HWND hwnd)
{
	assert(s_mouseInstance == nullptr);
	s_mouseInstance = new Mouse(hwnd);
}

void Mouse::Destroy()
{
	assert(s_mouseInstance);
	delete s_mouseInstance;
	s_mouseInstance = nullptr;
}

Mouse& Mouse::Get()
{
	assert(s_mouseInstance);
	return *s_mouseInstance;
}

void Mouse::Update()
{
	m_mouseState1 = m_mouseState0;
	m_mouseState0.LMBClicked = false;
	m_mouseState0.LMBReleased = false;
	m_mouseState0.RMBClicked = false;
	m_mouseState0.RMBReleased = false;
	m_mouseState0.MMBClicked = false;
	m_mouseState0.MMBReleased = false;
	m_mouseState0.deltaX = 0;
	m_mouseState0.deltaY = 0;
	m_mouseState0.deltaZ = 0;
}

MouseState Mouse::State() const
{
	return m_mouseState1;
}

void Mouse::SetMode(Mode mode)
{
	if ((m_mode & Mode::Visible) != (mode & Mode::Visible))
	{
		if ((mode & Mode::Visible) == Mode::Visible)
			int count = ShowCursor(true);
		else
			ShowCursor(false);
	}
	if ((m_mode & Mode::Confined) != (mode & Mode::Confined))
	{
		if ((mode & Mode::Confined) == Mode::Confined)
		{
			RECT r;
			GetClientRect(m_hWnd, &r);
			MapWindowPoints(m_hWnd, nullptr, (POINT*)&r, 2);
			ClipCursor(&r);
		}
		else
		{
			ClipCursor(nullptr);
		}
	}
	m_mode = mode;
	m_showCursor = (m_mode & Mode::Visible) == Mode::Visible;
}

Mouse::Mode Mouse::GetMode() const
{
	return m_mode;
};


Mouse::Mouse(HWND hwnd)
{
	m_hWnd = hwnd;
}
