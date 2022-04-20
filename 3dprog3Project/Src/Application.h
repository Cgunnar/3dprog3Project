#pragma once
#include "Window.h"

class Application
{
public:
	Application();
	~Application();

	void Run();
private:
	Window* m_window = nullptr;
};

