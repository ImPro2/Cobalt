#include "copch.hpp"
#include "Window.hpp"
#include <iostream>

namespace Cobalt
{

	Window::Window(uint32_t width, uint32_t height, const char* title)
		: mWidth(width), mHeight(height), mTitle(title)
	{
		CO_PROFILE_FN();
	}

	Window::~Window()
	{
		CO_PROFILE_FN();
	}

	void Window::Create()
	{
		CO_PROFILE_FN();

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

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double x, double y)
		{
			Window* self = (Window*)glfwGetWindowUserPointer(window);

			if (self->mMouseMoveCallback)
				self->mMouseMoveCallback((float)x, (float)y);
		});

	}

	void Window::Close()
	{
		CO_PROFILE_FN();

		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void Window::Update()
	{
		CO_PROFILE_FN();

		glfwPollEvents();
	}

}