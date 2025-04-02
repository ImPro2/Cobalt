#include "Application.hpp"

int main()
{
	Cobalt::Application app;

	app.Init();
	app.Run();
	app.Shutdown();
}