#include "Application.hpp"
#include "Vulkan/Renderer.hpp"

namespace Cobalt
{

	Application::Application()
	{
		if (sInstance)
			return;

		sInstance = this;

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

		Renderer::Init();
	}

	void Application::Run()
	{
		while (mRunning)
		{
			mWindow->Update();

			mGraphicsContext->RecreateSwapchainIfNeeded();

			mGraphicsContext->RenderFrame();
			mGraphicsContext->PresentFrame();
		}
	}

	void Application::Shutdown()
	{
		Renderer::Shutdown();

		mGraphicsContext->Shutdown();
		mWindow->Close();
	}

}