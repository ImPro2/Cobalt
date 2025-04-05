#pragma once
#include "GraphicsContext.hpp"
#include "Pipeline.hpp"
#include "VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <array>

namespace Cobalt
{

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static void BeginScene();
		static void EndScene();

		static void DrawTriangle();

	private:
		static void CreateOrRecreateFramebuffers();

	private:
		struct RendererData
		{
			VkRenderPass MainRenderPass;
			std::shared_ptr<Pipeline> TrianglePipeline;
			std::vector<VkFramebuffer> Framebuffers;
			std::unique_ptr<VulkanBuffer> VertexBuffer, IndexBuffer;
		};

		inline static RendererData* sData = nullptr;
	};

}
