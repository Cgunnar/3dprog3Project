#include "pch.h"

#include "Application.h"
#include "FrameTimer.h"

Application::Application()
{
	FrameTimer::Init();
	m_window = new Window();
	m_renderer = new Renderer(m_window->GetHWND());
}

Application::~Application()
{
	delete m_renderer;
	delete m_window;
}

void Application::Run()
{
	while (true)
	{
		FrameTimer::NewFrame();
		if (!m_window->Win32MsgPump()) break;
	}
}
