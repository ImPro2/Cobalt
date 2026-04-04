#pragma once
#include "VulkanUtils.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Cobalt
{

	class ImGuiPass
	{
	public:
		ImGuiPass();
		~ImGuiPass();

	public:
		void Init();
		void Shutdown();

		void BeginFrame();
		void RenderFrame(VkCommandBuffer commandBuffer);
		void EndFrame();

		void OnResize();

	private:
		void CreateOrRecreateFramebuffers();
		/*struct ImGuiBackendData
		{
			std::vector<VkCommandPool> CommandPools; // per frame
			std::vector<VkCommandBuffer> CommandBuffers; // per frame
			VkCommandBuffer ActiveCommandBuffer = VK_NULL_HANDLE;

			std::vector<VkFramebuffer> Framebuffers; // per backbuffer

			VkRenderPass ImGuiRenderPass = VK_NULL_HANDLE;
		};

		inline static ImGuiBackendData* sData = nullptr;*/

		std::vector<VkFramebuffer> mFramebuffers;
		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
	};

}