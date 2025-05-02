#include "Application.hpp"
#include "Vulkan/Renderer.hpp"
#include "Vulkan/ImGuiBackend.hpp"

namespace Cobalt
{

	Application::Application(ApplicationInfo&& info)
		: mInfo(std::move(info))
	{
		if (sInstance)
			return;

		sInstance = this;

		mWindow = std::make_unique<Window>();
		mWindow->OnWindowClose([this]() { this->OnWindowClose(); });
		mWindow->OnWindowResize([this](uint32_t width, uint32_t height) { this->OnWindowResize(width, height); });
		mWindow->OnMouseMove([this](float x, float y) { this->OnMouseMove(x, y); });

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

		if (mInfo.EnableImGui)
			ImGuiBackend::Init();

		for (Module* module : mModules)
			module->OnInit();
	}

	void Application::Run()
	{
		float lastFrameTime = 0.0f;

		while (mRunning)
		{
			float currentTime = glfwGetTime();
			float deltaTime = currentTime - lastFrameTime;
			lastFrameTime = currentTime;

			mWindow->Update();

			for (Module* module : mModules)
				module->OnUpdate(deltaTime);

			if (mGraphicsContext->ShouldRecreateSwapchain())
			{
				mGraphicsContext->OnResize();
				Renderer::OnResize();

				if (mInfo.EnableImGui)
					ImGuiBackend::OnResize();
			}

			if (mInfo.EnableImGui)
			{
				ImGuiBackend::BeginFrame();

				for (Module* module : mModules)
					module->OnUIRender();

				ImGuiBackend::EndFrame();
			}

			mGraphicsContext->RenderFrame(mModules);
			mGraphicsContext->PresentFrame();
		}
	}

	void Application::Shutdown()
	{
		for (Module* module : mModules)
		{
			module->OnShutdown();
			delete module;
			module = nullptr;
		}

		mModules.clear();

		vkDeviceWaitIdle(GraphicsContext::Get().GetDevice());

		ImGuiBackend::Shutdown();
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

	void Application::OnMouseMove(float x, float y)
	{
		for (Module* module : mModules)
			module->OnMouseMove(x, y);
	}

}