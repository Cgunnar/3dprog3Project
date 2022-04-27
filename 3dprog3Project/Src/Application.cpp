#include "pch.h"

#include "Application.h"
#include "FrameTimer.h"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "Mouse.h"
#include "RenderingTypes.h"
#include "CameraControllerScript.h"
#include "CommonComponents.h"
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
			m_renderer->FlushGPU();
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

			UINT width, height;
			if (m_window->GetFullscreenState() == FullscreenState::windowed)
			{
				std::tie(width, height) = Window::GetWidthAndHeight();
			}
			else
			{
				std::tie(width, height) = m_renderer->GetDisplayResolution();
			}

			std::vector<const char*> res =
			{
				"If you don't like pixels",
				"144",
				"360",
				"720",
				"1080",
				"1440",
				"2160",
				"4320",
				"If you like to go over the limit"
			};
			
			static int resIndex = 3;
			if(ImGui::Combo("resolution", &resIndex, res.data(), res.size()))
			{
				if (resIndex == 0)
					newSettings.renderHeight = 4;
				else if (resIndex == res.size() - 1)
					newSettings.renderHeight = 10000000000;
				else
					newSettings.renderHeight = std::stoi(res[resIndex]);
				newSettings.renderWidth = newSettings.renderHeight * width / height;
			}
			std::vector<const char*> numframes = { "1", "2", "3", "4", "5", "6"};
			static int numFramesIndex = 1;
			if (ImGui::Combo("frames in flight", &numFramesIndex, numframes.data(), numframes.size()))
			{
				newSettings.numberOfFramesInFlight = numFramesIndex + 1;
			}
			std::vector<const char*> numBackbuffers = { "2", "3", "4", "5", "6", "7" };
			static int numBackbuffersIndex = 2;
			if (ImGui::Combo("backbuffers", &numBackbuffersIndex, numBackbuffers.data(), numBackbuffers.size()))
			{
				newSettings.numberOfBackbuffers = numBackbuffersIndex + 2;
			}
			static bool applySettingsOnce = true; //should realy not do this, but this will give me the imgui settings as default init
			if (ImGui::Button("Apply") || applySettingsOnce)
			{
				applySettingsOnce = false;
				restartRenderer = true;
				runApplicationLoop = false;
				m_renderSettings = newSettings;
				
				//borderLess is not allowed when recreating the renderer
				if (m_window->GetFullscreenState() == FullscreenState::borderLess
					|| m_window->GetFullscreenState() == FullscreenState::fullscreen)
				{
					if (newSettings.fullscreemState == FullscreenState::windowed)
						m_window->SetFullscreen(FullscreenState::windowed);
					else
						m_window->SetFullscreen(FullscreenState::windowed, true, width, height);
				}
			}

			ImGui::End();

			if (Mouse::Get().State().RMBClicked)
			{
				Mouse::Get().SetMode(~Mouse::Get().GetMode());
				for (auto& c : rfe::EntityReg::ViewEntities<CameraControllerScript>())
					c.GetComponent<CameraControllerScript>()->ToggleCameraLock();
			}

			AssetManager::Get().Update(m_renderer->GetNumberOfFramesInFlight());
			for (auto& c : rfe::EntityReg::ViewEntities<CameraComp>())
			{
				c.GetComponent<CameraComp>()->projectionMatrix
					= rfm::PerspectiveProjectionMatrix(rfm::PIDIV4,
						static_cast<float>(width) / static_cast<float>(height), 0.001, 1000);
			}
			m_scene->Update(dt);

			ImGui::Render();
			m_renderer->Render();
		}
	}
}


void Application::SetUp()
{

}