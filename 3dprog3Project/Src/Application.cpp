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
	m_window = new Window();

	m_renderSettings.zPrePass = true;
	m_renderSettings.numberOfFramesInFlight = 3;
	m_renderSettings.numberOfBackbuffers = 3;
	m_renderSettings.renderHeight = 1080;

	auto [w, h] = m_window->GetWidthAndHeight();
	m_renderSettings.renderWidth = m_renderSettings.renderHeight * w / h;


	FrameTimer::Init();
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
			rfe::Entity camera = rfe::EntityReg::ViewEntities<CameraControllerScript, TransformComp>().front();
			rfm::Transform transform = camera.GetComponent<TransformComp>()->transform;
			float p = camera.GetComponent<CameraControllerScript>()->m_pitch;
			float y = camera.GetComponent<CameraControllerScript>()->m_yaw;
			camera.Reset(); //this holds a reference to the camera in the scene, cant hold it if we are to reset the scene without having created two cameras in the ecs

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

			camera = rfe::EntityReg::ViewEntities<CameraControllerScript, TransformComp>().front();
			camera.GetComponent<TransformComp>()->transform = transform;
			camera.GetComponent<CameraControllerScript>()->m_pitch = p;
			camera.GetComponent<CameraControllerScript>()->m_yaw = y;
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

			static RenderingSettings newSettings;
			if (frameNumber == 0) newSettings = m_renderSettings;
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

			

			UINT width, height;
			if (m_window->GetFullscreenState() == FullscreenState::windowed)
				std::tie(width, height) = Window::GetWidthAndHeight();
			else
				std::tie(width, height) = m_renderer->GetDisplayResolution();

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
			if (frameNumber == 0)
			{
				resIndex = [&]()->int {
					for (int i = 1; i < res.size() - 1; i++)
						if (newSettings.renderHeight == std::stoi(res[i])) return i;
					return 3; }();
			}

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
				m_renderSettings.renderWidth = newSettings.renderWidth;
				m_renderSettings.renderHeight = newSettings.renderHeight;
				bool succeeded = m_renderer->ChangeRenderingSettings(m_renderSettings);
				if (!succeeded)
				{
					utl::PrintDebug("ChangeRenderingSettings failed");
				}
			}

			if (ImGui::Checkbox("vsync", &newSettings.vsync))
			{
				m_renderSettings.vsync = newSettings.vsync;
				if (!m_renderer->ChangeRenderingSettings(m_renderSettings))
					utl::PrintDebug("ChangeRenderingSettings failed");
			}

			if (ImGui::Checkbox("shadows", &newSettings.shadows))
			{
				m_renderSettings.shadows = newSettings.shadows;
				if (!m_renderer->ChangeRenderingSettings(m_renderSettings))
					utl::PrintDebug("ChangeRenderingSettings failed");
			}

			std::vector<const char*> numBounces = { "0", "1", "2", "3", "4", "10", "12", "16", "20", "24", "32", "40", "50", "100"};
			static int numBouncesIndex = 0;
			if (frameNumber == 0)
			{
				numBouncesIndex = [&]()->int {
					for (int i = 1; i < numBounces.size() - 1; i++)
						if (newSettings.numberOfBounces == std::stoi(numBounces[i])) return i;
					return 0; }();
			}
			ImGui::Text("ray max bounces");
			ImGui::SameLine();
			if (ImGui::Combo("##3", &numBouncesIndex, numBounces.data(), static_cast<int>(numBounces.size())))
			{
				newSettings.numberOfBounces = std::stoi(numBounces[numBouncesIndex]);
				m_renderSettings.numberOfBounces = newSettings.numberOfBounces;
				if (!m_renderer->ChangeRenderingSettings(m_renderSettings))
					utl::PrintDebug("ChangeRenderingSettings failed");
			}

			ImGui::Separator();
			ImGui::Text("Restart renderer to apply");
			ImGui::Checkbox("Z prepass", &newSettings.zPrePass);
			ImGui::Checkbox("profiling", &profiling);
			std::vector<const char*> numframes = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19"};
			static int numFramesIndex = 1;
			if (frameNumber == 0) numFramesIndex = newSettings.numberOfFramesInFlight - 1;
			ImGui::Text("frames in flight");
			ImGui::SameLine();
			if (ImGui::Combo("##4", &numFramesIndex, numframes.data(), static_cast<int>(numframes.size())))
			{
				newSettings.numberOfFramesInFlight = numFramesIndex + 1;
			}
			std::vector<const char*> numBackbuffers = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
			static int numBackbuffersIndex = 0;
			if (frameNumber == 0) numBackbuffersIndex = newSettings.numberOfBackbuffers - 2;
			ImGui::Text("backbuffers");
			ImGui::SameLine();
			if (ImGui::Combo("##5", &numBackbuffersIndex, numBackbuffers.data(), static_cast<int>(numBackbuffers.size())))
			{
				newSettings.numberOfBackbuffers = numBackbuffersIndex + 2;
			}

			if (ImGui::Button("Restart renderer"))
			{
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