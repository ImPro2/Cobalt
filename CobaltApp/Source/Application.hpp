#pragma once
#include "Window.hpp"
#include "Vulkan/GraphicsContext.hpp"

#include <memory>

namespace Cobalt
{

	class Application
	{
	public:
		static Application* Get() { return sInstance; }

		const Window& GetWindow() const { return *mWindow; }

	public:
		Application();
		~Application();

	public:
		void Init();
		void Run();
		void Shutdown();

	private:
		inline static Application* sInstance;

		bool mRunning = true;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<GraphicsContext> mGraphicsContext;
	};

}
