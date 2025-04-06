#include "Application.hpp"
#include "SandboxModule.hpp"

int main()
{
	Cobalt::Application app;
	app.AddModule<Cobalt::SandboxModule>();

	app.Init();
	app.Run();
	app.Shutdown();
}