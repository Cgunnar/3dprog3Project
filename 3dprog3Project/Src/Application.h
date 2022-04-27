#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"

class Application
{
public:
	Application();
	~Application();
	Application(const Application& other) = delete;
	Application& operator=(const Application& other) = delete;
	void Run();
private:
	Window* m_window = nullptr;
	Renderer* m_renderer = nullptr;
	Scene* m_scene = nullptr;
	RenderingSettings m_renderSettings;

	void SetUp();
};

