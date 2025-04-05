#pragma once
#include <imgui.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Cobalt
{

	class ImGuiBackend
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame();    // called by Application
		static void RenderFrame(); // called by GraphicsContext

		static void OnResize();

	public:
		static VkCommandBuffer GetActiveCommandBuffer();

	private:
		static void CreateOrRecreateFramebuffers();

	private:
		struct ImGuiBackendData
		{
			std::vector<VkCommandPool>   CommandPools;   // per frame
			std::vector<VkCommandBuffer> CommandBuffers; // per frame

			std::vector<VkFramebuffer> Framebuffers; // per backbuffer

			VkRenderPass ImGuiRenderPass;
		};

		inline static ImGuiBackendData* sData = nullptr;
	};

}