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
		static GraphicsContext& Get() { return *sGraphicsContextInstance; }

		VkInstance       GetInstance()       const { return mInstance; }
		VkDevice         GetDevice()         const { return mDevice;   }
		VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }
		VkQueue          GetQueue()          const { return mQueue;    }

		const Swapchain& GetSwapchain() const { return *mSwapchain; }

		bool ShouldRecreateSwapchain() const { return mRecreateSwapchain; }

	public:
		VkCommandBuffer AllocateTransientCommandBuffer();
		void SubmitSingleTimeCommands(VkQueue queue, std::function<void(VkCommandBuffer)> fn);

	public:
		GraphicsContext(const Window& window);
		~GraphicsContext();

	public:
		void Init();
		void Shutdown();

		void RenderFrame();
		void PresentFrame();

		void OnResize();

	public:
		VkCommandBuffer GetActiveCommandBuffer() const { return mActiveCommandBuffer; }

	private:
		const Window& mWindow;

		uint32_t mFrameCount = 2;
		uint32_t mFrameIndex = 0;
		std::vector<FrameData> mFrames;

		VkCommandBuffer mActiveCommandBuffer;

	private:
		inline static GraphicsContext* sGraphicsContextInstance = nullptr;

		VkInstance               mInstance;
		VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
		VkPhysicalDevice         mPhysicalDevice;
		int32_t                  mQueueFamily;
		VkQueue                  mQueue;
		VkDevice                 mDevice;
		VkDescriptorPool         mDescriptorPool;
		VkSurfaceKHR             mSurface;
		VkCommandPool            mTransientCommandPool;

		std::unique_ptr<Swapchain> mSwapchain;

		bool mRecreateSwapchain = false;
		bool mEnableValidationLayers = true;
	};

}
