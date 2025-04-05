#include "Window.hpp"
#include <iostream>

namespace Cobalt
{

	Window::Window(uint32_t width, uint32_t height, const char* title)
		: mWidth(width), mHeight(height), mTitle(title)
	{
	}

	Window::~Window()
	{
	}

	void Window::Create()
	{
		int32_t status = glfwInit();

		if (status != GLFW_TRUE)
		{
			std::cerr << "Error: Failed to initialize GLFW.";
			std::exit(1);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(mWindow, this);

		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window)
		{
			Window* self = (Window*)glfwGetWindowUserPointer(window);

			if (self->mCloseCallback)
				self->mCloseCallback();
		});

		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			Window* self = (Window*)glfwGetWindowUserPointer(window);
			self->mWidth = (uint32_t)width;
			self->mHeight = (uint32_t)height;

			if (self->mResizeCallback)
				self->mResizeCallback(self->mWidth, self->mHeight);
		});
	}

	void Window::Close()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void Window::Update()
	{
		glfwPollEvents();
	}

}