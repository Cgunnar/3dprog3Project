#include "pch.h"

#include "Application.h"
int main()
{
	std::cout << "Start" << std::endl;
	Application* myApp = new Application();
	delete myApp;
	std::cout << "Exit" << std::endl;
	return 0;
}