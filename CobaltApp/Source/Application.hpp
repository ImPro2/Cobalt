#pragma once
#include "Window.hpp"
#include "Vulkan/GraphicsContext.hpp"
#include "Module.hpp"

#include <memory>

namespace Cobalt
{

	struct ApplicationInfo
	{
		bool EnableImGui = true;
		bool EnableOptickCapture = false;
	};

	class Application
	{
	public:
		static Application* Get() { return sInstance; }
		const ApplicationInfo& GetInfo() const { return mInfo; }

		const Window& GetWindow() const { return *mWindow; }

	public:
		Application(ApplicationInfo&& info);
		~Application();

	public:
		template<typename T>
		Module* AddModule()
		{
			static_assert(std::is_base_of_v<Module, T>);

			Module* module = new T;
			mModules.push_back(module);

			return module;
		}

	public:
		void Init();
		void Run();
		void Shutdown();

	private:
		void OnWindowClose();
		void OnWindowResize(uint32_t width, uint32_t height);
		void OnMouseMove(float x, float y);

	private:
		inline static Application* sInstance;

		ApplicationInfo mInfo;

		bool mRunning = true;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<GraphicsContext> mGraphicsContext;

		std::vector<Module*> mModules;
	};

}
