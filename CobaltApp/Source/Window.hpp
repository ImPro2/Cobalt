#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <string>

namespace Cobalt
{

	class Window
	{
	public:
		using CloseCallback = std::function<void()>;
		using ResizeCallback = std::function<void(uint32_t, uint32_t)>;

	public:
		Window(uint32_t width = 1024, uint32_t height = 800, const char* title = "Cobalt App");
		~Window();

	public:
		void Create();
		void Close();

		void Update();

	public:
		// called by Application
		void OnWindowClose(CloseCallback callback) { mCloseCallback = callback; }
		void OnWindowResize(ResizeCallback callback) { mResizeCallback = callback; }

	public:
		GLFWwindow*        GetWindow() const { return mWindow; }
		uint32_t           GetWidth()  const { return mWidth;  }
		uint32_t           GetHeight() const { return mHeight; }
		const std::string& GetTitle()  const { return mTitle;  }

	private:
		GLFWwindow* mWindow = nullptr;

		uint32_t    mWidth;
		uint32_t    mHeight;
		std::string mTitle;

		CloseCallback mCloseCallback;
		ResizeCallback mResizeCallback;
	};

}
