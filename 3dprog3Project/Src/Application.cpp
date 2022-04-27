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
	m_renderer = new Renderer(m_window->GetHWND(), m_renderSettings);
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
			m_renderer = new Renderer(m_window->GetHWND(), m_renderSettings);
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

			static RenderingSettings newSettings;
			ImGui::Begin("Settings");
			if (ImGui::Button("Windowed"))
				newSettings.fullscreemState = FullscreenState::windowed;
			if (ImGui::Button("Borderless"))
				newSettings.fullscreemState = FullscreenState::borderLess;
			if (ImGui::Button("Fullscreen"))
				newSettings.fullscreemState = FullscreenState::fullscreen;
			if (m_renderSettings.fullscreemState != newSettings.fullscreemState)
			{
				m_window->SetFullscreen(newSettings.fullscreemState);
				m_renderSettings.fullscreemState = newSettings.fullscreemState;
			}
			static bool vsync = false;
			if (ImGui::Checkbox("vsync", &vsync))
				m_renderer->vsyncEnabled = vsync;

			const char* res[] =
			{
				"720",
				"1440"
			};
			static int resIndex = 0;
			if(ImGui::Combo("select res", &resIndex, res, 2))
			{
				std::cout << res[resIndex] << std::endl;
				switch (resIndex)
				{
				case 0:
					newSettings.renderWidth = 1280;
					newSettings.renderHeight = 720;
					break;
				case 1:
					newSettings.renderWidth = 2560;
					newSettings.renderHeight = 1440;
					break;
				default:
					break;
				}
			}

			if (ImGui::Button("Apply"))
			{
				restartRenderer = true;
				runApplicationLoop = false;
				m_renderSettings = newSettings;
				if (m_renderSettings.fullscreemState == FullscreenState::borderLess);
					m_window->SetFullscreen(FullscreenState::windowed);
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