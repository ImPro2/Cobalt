#include "Application.hpp"

namespace Cobalt
{

	Application::Application()
	{
		mWindow = std::make_unique<Window>([this]()
		{
			mRunning = false;
		});
	}

	Application::~Application()
	{
	}

	void Application::Init()
	{
		mWindow->Create();
	}

	void Application::Run()
	{
		while (mRunning)
		{
			mWindow->Update();
		}
	}

	void Application::Shutdown()
	{
		mWindow->Close();
	}

}