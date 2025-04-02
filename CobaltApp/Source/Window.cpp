#include "Window.hpp"
#include <iostream>

namespace Cobalt
{

	Window::Window(CloseCallback closeCallback, uint32_t width, uint32_t height, const char* title)
		: mCloseCallback(closeCallback), mWidth(width), mHeight(height), mTitle(title)
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

		mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(mWindow, this);

		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window)
		{
			Window* self = (Window*)glfwGetWindowUserPointer(window);
			self->mCloseCallback();
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