#pragma once
#include "Window.hpp"
#include "Vulkan/GraphicsContext.hpp"

#include <memory>

namespace Cobalt
{

	class Application
	{
	public:
		Application();
		~Application();

	public:
		void Init();
		void Run();
		void Shutdown();

	private:
		bool mRunning = true;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<GraphicsContext> mGraphicsContext;
	};

}
