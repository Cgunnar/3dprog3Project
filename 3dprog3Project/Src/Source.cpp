#include "pch.h"

#include "Application.h"
int main()
{
	std::cout << "main start" << std::endl;
	Application* myApp = new Application();
	myApp->Run();
	delete myApp;
	std::cout << "main exit" << std::endl;
	return 0;
}