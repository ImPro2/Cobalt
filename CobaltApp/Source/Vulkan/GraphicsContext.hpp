#pragma once
#include "../Window.hpp"
#include "Swapchain.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <memory>

namespace Cobalt
{

	struct FrameData
	{
		VkCommandPool   CommandPool;
		VkCommandBuffer CommandBuffer;

		VkFence     AcquireNextImageFence;
		VkSemaphore ImageAcquiredSemaphore;
		VkSemaphore RenderFinishedSemaphore;
	};

	class GraphicsContext
	{
	public:
		GraphicsContext(const Window& window);
		~GraphicsContext();

	public:
		void Init();
		void Shutdown();

		void RenderFrame();
		void PresentFrame();

		void RecreateSwapchainIfNeeded();

	public:
		VkCommandBuffer GetActiveCommandBuffer() const { return mActiveCommandBuffer; }

	private:
		void CreateOrRecreateFramebuffers();

	private:
		const Window& mWindow;

		uint32_t mFrameCount = 2;
		uint32_t mFrameIndex = 0;
		std::vector<FrameData> mFrames;

		uint32_t mBackbufferIndex = 0;

		VkCommandBuffer mActiveCommandBuffer;

	private:
		VkInstance       mInstance;
		VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
		VkPhysicalDevice mPhysicalDevice;
		int32_t          mQueueFamily;
		VkQueue          mQueue;
		VkDevice         mDevice;
		VkDescriptorPool mDescriptorPool;
		VkSurfaceKHR     mSurface;

		std::unique_ptr<Swapchain> mSwapchain;

		bool mRecreateSwapchain = false;

		VkRenderPass mRenderPass;

		std::vector<VkFramebuffer> mFramebuffers;

		bool mEnableValidationLayers = true;
	};

}
