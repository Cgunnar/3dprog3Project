#include "pch.h"

#include "Application.h"
#include "FrameTimer.h"
#include "rfEntity.hpp"
#include "AssetManager.h"
#include "Mouse.h"
#include "KeyBoard.h"
#include "RenderingTypes.h"
#include "CameraControllerScript.h"
#include "CommonComponents.h"
#include "Timer.hpp"
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

Application::Application()
{
	FrameTimer::Init();
	m_window = new Window();
	m_renderer = new Renderer(m_window->GetHWND(), m_renderSettings);
	m_window->SetRenderer(m_renderer);
	AssetManager::Init(m_renderer);
	m_scene = new Scene();
	m_renderer->PostAssetManagerSetUp();
}

Application::~Application()
{
	m_renderer->FlushGPU();
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
		Mouse::Get().SetMode(Mouse::Mode::Visible);
		if (restartRenderer)
		{
			delete m_scene;
			m_renderer->FlushGPU();
			AssetManager::Destroy();
			m_window->SetRenderer(nullptr);
			delete m_renderer;
			m_renderer = new Renderer(m_window->GetHWND(), m_renderSettings);
			m_window->SetRenderer(m_renderer);
			AssetManager::Init(m_renderer);
			m_scene = new Scene();
			m_renderer->PostAssetManagerSetUp();
			restartRenderer = false;
			Mouse::Get().SetMode(Mouse::Mode::Visible);
		}

		Timer myTimer(Duration::NANOSECONDS);
		uint64_t frameNumber = 0;
		bool runApplicationLoop = true;
		while (runApplicationLoop)
		{
			float dt = static_cast<float>(FrameTimer::NewFrame());
			Mouse::Get().Update();
			KeyBoard::Get().Update();
			static uint64_t frameCounter = 0;
			static uint64_t avgFramesFrameTimeNanoSec = 0;
			static uint64_t avgOver2000FramesFrameTimeNanoSec = 0;
			static bool profiling = false;
			if (profiling)
			{
				if (frameNumber == 500)
				{
					myTimer.start();
					frameCounter = 0;
					avgFramesFrameTimeNanoSec = 0;
					avgOver2000FramesFrameTimeNanoSec = 0;
					std::cout << "start profiling" << std::endl;
				}
				if (frameNumber > 500) // wait a bit before staring profiling
				{
					uint64_t time = myTimer.stop();
					myTimer.start();
					avgOver2000FramesFrameTimeNanoSec += time;
					avgFramesFrameTimeNanoSec += time;
					if (frameCounter % 2000 == 0 && frameCounter)
					{
						uint64_t avg2000 = avgOver2000FramesFrameTimeNanoSec / 2000;
						uint64_t avgTotal = avgFramesFrameTimeNanoSec / frameCounter;
						avgOver2000FramesFrameTimeNanoSec = 0;
						std::cout << "runtime: " << std::to_string(FrameTimer::TimeFromLaunch()) << " s\n";
						std::cout << "frame time:\n";
						std::cout << "\tavg(2000):\t" + std::to_string(static_cast<double>(avg2000) / 1.0E+6) + " ms\n";
						std::cout << "\tavg(total):\t" + std::to_string(static_cast<double>(avgTotal) / 1.0E+6) + " ms" << std::endl;
					}
					frameCounter++;
				}
			}
			if (!m_window->Win32MsgPump())
			{
				runApplication = false;
				break;
			}
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//ImGui::ShowDemoWindow();

			static RenderingSettings newSettings;
			newSettings.fullscreemState = m_window->GetFullscreenState();
			ImGui::Begin("Settings");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SameLine();
			static bool pauseSceneUpdate = true;
			if (ImGui::Button("pause scene")) pauseSceneUpdate = !pauseSceneUpdate;
			std::array<const char*, 3> fullscreenCompo = {"windowed", "borderLess", "fullscreen"};
			FullscreenState selectedFullscreenState = newSettings.fullscreemState;
			ImGui::Text("fullscreen mode");
			ImGui::SameLine();
			ImGui::Combo("##1", reinterpret_cast<int*>(&newSettings.fullscreemState), fullscreenCompo.data(), static_cast<int>(fullscreenCompo.size()));
			if (m_renderSettings.fullscreemState != newSettings.fullscreemState)
			{
				m_window->SetFullscreen(newSettings.fullscreemState);
				m_renderSettings.fullscreemState = newSettings.fullscreemState;
			}

			if (ImGui::Checkbox("vsync", &newSettings.vsync))
			{
				m_renderer->SetVSync(newSettings.vsync);
				m_renderSettings.vsync = newSettings.vsync;
			}

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
			ImGui::Text("resolution");
			ImGui::SameLine();
			if(ImGui::Combo("##2", &resIndex, res.data(), static_cast<int>(res.size())))
			{
				if (resIndex == 0)
					newSettings.renderHeight = 4;
				else if (resIndex == res.size() - 1)
					newSettings.renderHeight = 1000000000u;
				else
					newSettings.renderHeight = std::stoi(res[resIndex]);
				newSettings.renderWidth = newSettings.renderHeight * width / height;
				m_renderer->SetRenderResolution(newSettings.renderWidth, newSettings.renderWidth);
			}
			std::vector<const char*> numframes = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19"};
			static int numFramesIndex = 1;
			ImGui::Separator();
			ImGui::Text("Restart renderer to apply");
			ImGui::Checkbox("profiling", &profiling);
			ImGui::Text("frames in flight");
			ImGui::SameLine();
			if (ImGui::Combo("##3", &numFramesIndex, numframes.data(), static_cast<int>(numframes.size())))
			{
				newSettings.numberOfFramesInFlight = numFramesIndex + 1;
			}
			std::vector<const char*> numBackbuffers = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
			static int numBackbuffersIndex = 0;
			ImGui::Text("backbuffers");
			ImGui::SameLine();
			if (ImGui::Combo("##4", &numBackbuffersIndex, numBackbuffers.data(), static_cast<int>(numBackbuffers.size())))
			{
				newSettings.numberOfBackbuffers = numBackbuffersIndex + 2;
			}
			static bool applySettingsOnce = true; //should realy not do this, but this will give me the imgui settings as default init
			if (ImGui::Button("Restart renderer") || applySettingsOnce)
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
			}
			if ((Mouse::Get().GetMode() & Mouse::Mode::Visible) == Mouse::Mode::Visible)
			{
				for (auto& c : rfe::EntityReg::ViewEntities<CameraControllerScript>())
				{
					auto cs = c.GetComponent<CameraControllerScript>();
					if (!cs->IsCameraLocked()) cs->ToggleCameraLock();
				}
			}
			else
			{
				for (auto& c : rfe::EntityReg::ViewEntities<CameraControllerScript>())
				{
					auto cs = c.GetComponent<CameraControllerScript>();
					if (cs->IsCameraLocked()) cs->ToggleCameraLock();
				}
			}

			AssetManager::Get().Update(m_renderer->GetNumberOfFramesInFlight());
			for (auto& c : rfe::EntityReg::ViewEntities<CameraComp>())
			{
				c.GetComponent<CameraComp>()->projectionMatrix
					= rfm::PerspectiveProjectionMatrix(rfm::PIDIV4,
						static_cast<float>(width) / static_cast<float>(height), 0.001f, 1000);
			}
			rfe::EntityReg::RunScripts<CameraControllerScript>(dt);
			if(!pauseSceneUpdate)
				m_scene->Update(dt);

			ImGui::Render();
			frameNumber = m_renderer->Render();
		}
	}
}


void Application::SetUp()
{

}