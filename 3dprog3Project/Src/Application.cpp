#include "pch.h"

#include "Application.h"
#include "FrameTimer.h"
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

Application::Application()
{
	FrameTimer::Init();
	m_window = new Window();
	m_renderer = new Renderer(m_window->GetHWND());
	m_window->SetRenderer(m_renderer);
}

Application::~Application()
{
	m_window->SetRenderer(nullptr);
	delete m_renderer;
	delete m_window;
}

void Application::Run()
{
	while (true)
	{
		FrameTimer::NewFrame();
		if (!m_window->Win32MsgPump()) break;
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();

		
		m_renderer->Render();
	}
}
