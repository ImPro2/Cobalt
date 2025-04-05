#include "Application.hpp"
#include "Vulkan/Renderer.hpp"

namespace Cobalt
{

	Application::Application()
	{
		if (sInstance)
			return;

		sInstance = this;

		mWindow = std::make_unique<Window>();
		mWindow->OnWindowClose([this]() { this->OnWindowClose(); });
		mWindow->OnWindowResize([this](uint32_t width, uint32_t height) { this->OnWindowResize(width, height); });

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

			if (mGraphicsContext->ShouldRecreateSwapchain())
			{
				mGraphicsContext->OnResize();
				Renderer::OnResize();
			}

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

	void Application::OnWindowClose()
	{
		mRunning = false;
	}

	void Application::OnWindowResize(uint32_t width, uint32_t height)
	{
	}

}