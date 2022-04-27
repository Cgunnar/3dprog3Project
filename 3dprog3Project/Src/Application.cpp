#include "pch.h"

#include "Application.h"
#include "FrameTimer.h"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "Mouse.h"
#include "RenderingTypes.h"
#include "CameraControllerScript.h"
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

Application::Application()
{
	FrameTimer::Init();
	m_window = new Window();
	m_renderer = new Renderer(m_window->GetHWND(), RenderingSettings());
	m_window->SetRenderer(m_renderer);
	AssetManager::Init(m_renderer->GetDevice());
	m_scene = new Scene();
}

Application::~Application()
{
	delete m_scene;
	AssetManager::Destroy();
	m_window->SetRenderer(nullptr);
	delete m_renderer;
	delete m_window;


	rfe::EntityReg::Clear();
}


void Application::Run()
{
	bool runApplication = true;
	bool restartRenderer = false;
	while (runApplication)
	{
		Mouse::Get().SetMode(Mouse::Mode::Confined);
		if (restartRenderer)
		{
			delete m_scene;
			AssetManager::Destroy();
			m_window->SetRenderer(nullptr);
			delete m_renderer;
			m_renderer = new Renderer(m_window->GetHWND(), RenderingSettings());
			m_window->SetRenderer(m_renderer);
			AssetManager::Init(m_renderer->GetDevice());
			m_scene = new Scene();
			restartRenderer = false;
		}

		bool runApplicationLoop = true;
		while (runApplicationLoop)
		{
			float dt = FrameTimer::NewFrame();
			Mouse::Get().Update();
			if (!m_window->Win32MsgPump())
			{
				runApplication = false;
				break;
			}
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::ShowDemoWindow();

			ImGui::Begin("Settings");
			if (ImGui::Button("Windowed"))
				m_window->SetFullscreen(Window::FullscreenState::windowed);
			if (ImGui::Button("Borderless"))
				m_window->SetFullscreen(Window::FullscreenState::borderLess);
			if (ImGui::Button("Fullscreen"))
				m_window->SetFullscreen(Window::FullscreenState::fullscreen);
			static bool vsync = false;
			if (ImGui::Checkbox("vsync", &vsync))
				m_renderer->vsyncEnabled = vsync;

			if (ImGui::Button("Restart renderer"))
			{
				restartRenderer = true;
				runApplicationLoop = false;
				m_window->SetFullscreen(Window::FullscreenState::windowed);
			}

			ImGui::End();

			if (Mouse::Get().State().RMBClicked)
			{
				Mouse::Get().SetMode(~Mouse::Get().GetMode());
				for (auto& c : rfe::EntityReg::ViewEntities<CameraControllerScript>())
					c.GetComponent<CameraControllerScript>()->ToggleCameraLock();
			}

			AssetManager::Get().Update(m_renderer->GetNumberOfFramesInFlight());
			m_scene->Update(dt);

			ImGui::Render();
			m_renderer->Render();
		}
	}
}


void Application::SetUp()
{

}