#include "pch.h"

#include "Application.h"

Application::Application()
{
	m_window = new Window();
}

Application::~Application()
{
	delete m_window;
}

void Application::Run()
{
	while (true)
	{
		if (!m_window->Win32MsgPump()) break;
	}
}
