#include "Application.hpp"

namespace Cobalt
{

	Application::Application()
	{
		mWindow = std::make_unique<Window>([this]()
		{
			mRunning = false;
		});

		mGraphicsContext = std::make_unique<GraphicsContext>(*mWindow);
	}

	Application::~Application()
	{
	}

	void Application::Init()
	{
		mWindow->Create();
		mGraphicsContext->Init();
	}

	void Application::Run()
	{
		while (mRunning)
		{

			// Update

			mGraphicsContext->RecreateSwapchainIfNeeded();

			mGraphicsContext->RenderFrame();
			mGraphicsContext->PresentFrame();

			mWindow->Update();
		}
	}

	void Application::Shutdown()
	{
		mWindow->Close();
	}

}