#pragma once
#include "Window.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.h>

namespace Cobalt
{

	class Swapchain
	{
	public:
		Swapchain(const Window& window, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		~Swapchain();

	public:
		void Recreate();

	public:
		VkSwapchainKHR GetHandle() const { return mSwapchain; }

		const VkSurfaceFormatKHR GetSurfaceFormat() const { return mSurfaceFormat; }
		const VkPresentModeKHR   GetPresentMode()   const { return mPresentMode;   }
		const VkExtent2D         GetExtent()        const { return mExtent;        }

		uint32_t           GetBackBufferCount() const { return mBackBufferCount; }
		const VkImage*     GetBackBuffers()     const { return mBackBuffers;     }
		const VkImageView* GetBackBufferViews() const { return mBackBufferViews; }

		uint32_t        GetBackBufferIndex()    const { return mBackBufferIndex;  }
		uint32_t* GetBackBufferIndexPtr() const { return (uint32_t*)&mBackBufferIndex; }

	private:
		void CreateOrRecreateSwapchain();
		void CreateOrRecreateBackbuffers();

	private:
		const Window&      mWindow;
		VkDevice           mDevice;
		VkPhysicalDevice   mPhysicalDevice;
		VkSurfaceKHR       mSurface;

		VkSwapchainKHR     mSwapchain = VK_NULL_HANDLE;

		VkSurfaceFormatKHR mSurfaceFormat;
		VkPresentModeKHR   mPresentMode;
		VkExtent2D         mExtent;

		uint32_t           mBackBufferCount = 0;
		VkImage*           mBackBuffers     = nullptr;
		VkImageView*       mBackBufferViews = nullptr;

		uint32_t           mBackBufferIndex = 0;
	};

}
