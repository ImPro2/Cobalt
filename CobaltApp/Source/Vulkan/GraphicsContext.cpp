#include "GraphicsContext.hpp"
#include "VulkanUtils.hpp"

#include <cstdlib>
#include <iostream>


namespace Cobalt
{

	namespace Utils
	{


	}

	GraphicsContext::GraphicsContext(const Window& window)
		: mWindow(window)
	{
	}

	GraphicsContext::~GraphicsContext()
	{
	}

	void GraphicsContext::Init()
	{
		// Setup Vulkan Instance

		{
			uint32_t extensionCount = 0;
			const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
			const char** extensions;

			if (mEnableValidationLayers)
			{
				extensionCount++;
				extensions = (const char**)malloc(sizeof(const char*) * extensionCount);
				memcpy(extensions, requiredExtensions, extensionCount * sizeof(const char*));
				extensions[extensionCount - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			}
			else
			{
				extensions = requiredExtensions;
			}

			const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

			VkApplicationInfo applicationInfo = {
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Cobalt Application",
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "Cobalt",
				.engineVersion = VK_MAKE_VERSION(1, 0, 0),
				.apiVersion = VK_API_VERSION_1_0
			};

			VkInstanceCreateInfo instanceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.flags = 0,
				.pApplicationInfo = &applicationInfo,
				.enabledExtensionCount = extensionCount,
				.ppEnabledExtensionNames = extensions
			};

			if (mEnableValidationLayers)
			{
				instanceCreateInfo.ppEnabledLayerNames = layers;
				instanceCreateInfo.enabledLayerCount = 1;
			}
			else
			{
				instanceCreateInfo.enabledExtensionCount = extensionCount;
				instanceCreateInfo.ppEnabledExtensionNames = extensions;
			}

			VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &mInstance));

			if (mEnableValidationLayers)
			{
				free(extensions);
			}
		}

		// Create debug utils messenger

		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
					.pfnUserCallback = VulkanDebugCallback
			};

			auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
			VK_CALL(fn(mInstance, &createInfo, nullptr, &mDebugUtilsMessenger));
		}

		// Select physical device

		{
			uint32_t count;
			VK_CALL(vkEnumeratePhysicalDevices(mInstance, &count, nullptr));

			VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)malloc(count * sizeof(VkPhysicalDevice));
			VK_CALL(vkEnumeratePhysicalDevices(mInstance, &count, physicalDevices));

			uint32_t selectedIndex = 0;

			for (uint32_t i = 0; i < count; i++)
			{
				VkPhysicalDeviceProperties properties;
				vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

				if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					selectedIndex = i;
					break;
				}
			}

			mPhysicalDevice = physicalDevices[selectedIndex];

			free(physicalDevices);
		}

		// Select graphics queue family

		{
			uint32_t count;
			vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &count, nullptr);

			VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(count * sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &count, queues);

			for (int32_t i = 0; i < count; i++)
			{
				if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					mQueueFamily = i;
					break;
				}
			}
			
			free(queues);
		}

		// Create logical device

		{
			const char* deviceExtensions[] = { "VK_KHR_swapchain" };
			const float queuePriority[] = { 1.0f };

			VkDeviceQueueCreateInfo queueCreateInfo[1] = {
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = (uint32_t)mQueueFamily,
					.queueCount = 1,
					.pQueuePriorities = queuePriority
				}
			};

			VkDeviceCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = queueCreateInfo,
				.enabledExtensionCount = 1,
				.ppEnabledExtensionNames = deviceExtensions,
			};

			VK_CALL(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));

			vkGetDeviceQueue(mDevice, mQueueFamily, 0, &mQueue);
		}

		// Create descriptor pool
		
		{
			VkDescriptorPoolSize poolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
				.maxSets = 1000 * (sizeof(poolSizes) / sizeof(poolSizes[0])),
				.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
				.pPoolSizes = poolSizes
			};

			VK_CALL(vkCreateDescriptorPool(mDevice, &createInfo, nullptr, &mDescriptorPool));
		}

		// Create window surface

		{
			VK_CALL(glfwCreateWindowSurface(mInstance, mWindow.GetWindow(), nullptr, &mSurface));
		}

		// Create window swapchain

		{
			mSwapchain = std::make_unique<Swapchain>(mWindow, mDevice, mPhysicalDevice, mSurface);
		}

		// Create the Render Pass

		{
			VkAttachmentDescription attachment = {};
			attachment.format = mSwapchain->GetSurfaceFormat().format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			VkAttachmentReference color_attachment = {};
			color_attachment.attachment = 0;
			color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment;
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			VK_CALL(vkCreateRenderPass(mDevice, &info, nullptr, &mRenderPass));
		}

		// Create framebuffers

		{
			CreateOrRecreateFramebuffers();
		}

		// Create frames

		{
			mFrames.resize(mFrameCount);

			for (FrameData& fd : mFrames)
			{
				// Create command pool

				{
					VkCommandPoolCreateInfo commandPoolCreateInfo = {
						.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
						.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
						.queueFamilyIndex = (uint32_t)mQueueFamily,
					};

					VK_CALL(vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &fd.CommandPool));
				}

				// Allocate command buffer

				{
					VkCommandBufferAllocateInfo commandBufferAllocInfo = {
						.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
						.commandPool = fd.CommandPool,
						.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
						.commandBufferCount = 1,
					};

					VK_CALL(vkAllocateCommandBuffers(mDevice, &commandBufferAllocInfo, &fd.CommandBuffer));
				}

				// Create synchronisation objects 

				{
					VkFenceCreateInfo fenceCreateInfo = {
						.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
						.flags = VK_FENCE_CREATE_SIGNALED_BIT
					};

					VkSemaphoreCreateInfo semaphoreCreateInfo = {
						.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
						.flags = 0
					};

					VK_CALL(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &fd.AcquireNextImageFence));

					VK_CALL(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &fd.ImageAcquiredSemaphore));
					VK_CALL(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &fd.RenderFinishedSemaphore));
				}
			}
		}
	}

	void GraphicsContext::Shutdown()
	{
		vkDestroyInstance(mInstance, nullptr);
	}

	void GraphicsContext::RenderFrame()
	{
		VkResult result;

		const FrameData& fd = mFrames[mFrameIndex];

		// Acquire next image

		{
			VK_CALL(vkWaitForFences(mDevice, 1, &fd.AcquireNextImageFence, VK_TRUE, UINT64_MAX));

			result = vkAcquireNextImageKHR(mDevice, mSwapchain->GetHandle(), UINT64_MAX, fd.ImageAcquiredSemaphore, VK_NULL_HANDLE, &mBackbufferIndex);

			if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || mBackbufferIndex == -1)
			{
				mRecreateSwapchain = true;
				return;
			}

			VK_CALL(result);

			VK_CALL(vkResetFences(mDevice, 1, &fd.AcquireNextImageFence));
		}

		// Start command buffer

		{
			VK_CALL(vkResetCommandPool(mDevice, fd.CommandPool, 0));

			VkCommandBufferBeginInfo beginInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
			};

			VK_CALL(vkBeginCommandBuffer(fd.CommandBuffer, &beginInfo));
			mActiveCommandBuffer = fd.CommandBuffer;
		}

		// Do rendering

		{
			VkClearValue clearValues = {.color = {{1.0f, 0.0f, 0.0f, 1.0f}}};

			VkRenderPassBeginInfo beginInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = mRenderPass,
				.framebuffer = mFramebuffers[mBackbufferIndex],
				.renderArea = {.extent = mSwapchain->GetExtent() },
				.clearValueCount = 1,
				.pClearValues = &clearValues,
			};

			vkCmdBeginRenderPass(fd.CommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// ...

			vkCmdEndRenderPass(fd.CommandBuffer);
		}

		// Submit command buffer

		{
			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo submitInfo = {
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &fd.ImageAcquiredSemaphore,
				.pWaitDstStageMask = &waitStage,
				.commandBufferCount = 1,
				.pCommandBuffers = &fd.CommandBuffer,
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &fd.RenderFinishedSemaphore
			};

			VK_CALL(vkEndCommandBuffer(fd.CommandBuffer));
			VK_CALL(vkQueueSubmit(mQueue, 1, &submitInfo, fd.AcquireNextImageFence));

			mActiveCommandBuffer = VK_NULL_HANDLE;
		}
	}

	void GraphicsContext::PresentFrame()
	{
		if (mRecreateSwapchain)
			return;

		VkSwapchainKHR swapchain = mSwapchain->GetHandle();

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &mFrames[mFrameIndex].RenderFinishedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &mBackbufferIndex
		};

		VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);

		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			mRecreateSwapchain = true;
			return;
		}

		VK_CALL(result);

		mFrameIndex = (mFrameIndex + 1) % mFrameCount;
	}

	void GraphicsContext::RecreateSwapchainIfNeeded()
	{
		if (mRecreateSwapchain)
		{
			mRecreateSwapchain = false;

			int32_t width, height;
			glfwGetFramebufferSize(mWindow.GetWindow(), &width, &height);

			if (width == 0 || height == 0)
				return;

			mSwapchain->Recreate();
			CreateOrRecreateFramebuffers();
		}
	}

	void GraphicsContext::CreateOrRecreateFramebuffers()
	{
		if (mFramebuffers.size() > 0)
		{
			for (VkFramebuffer framebuffer : mFramebuffers)
				vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

			mFramebuffers.clear();
		}

		mFramebuffers.resize(mSwapchain->GetBackBufferCount());

		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.flags = 0,
			.renderPass = mRenderPass,
			.attachmentCount = 1,
			.width = mSwapchain->GetExtent().width,
			.height = mSwapchain->GetExtent().height,
			.layers = 1,
		};

		for (uint32_t i = 0; i < mSwapchain->GetBackBufferCount(); i++)
		{
			VkImageView attachment[1] = { mSwapchain->GetBackBufferViews()[i] };
			createInfo.pAttachments = attachment;

			VK_CALL(vkCreateFramebuffer(mDevice, &createInfo, nullptr, &mFramebuffers[i]));
		}
	}

}