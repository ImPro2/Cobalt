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

		static void DrawSquare();

	private:
		static void CreateOrRecreateFramebuffers();

	private:
		struct RendererData
		{
			VkRenderPass MainRenderPass;
			std::shared_ptr<Pipeline> TrianglePipeline;
			std::vector<VkFramebuffer> Framebuffers;
			std::unique_ptr<VulkanBuffer> VertexBuffer, IndexBuffer;

			VkImage DepthTexture;
			VkImageView DepthTextureView;
			VkDeviceMemory DepthTextureMemory;

			glm::vec3 CameraPosition = glm::vec3(0, 0, 3);
		};

		struct PushConstants
		{
			glm::mat4 ViewProjection;
			glm::mat4 Transform;
		};

		inline static RendererData* sData = nullptr;
	};

}
