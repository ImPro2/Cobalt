#pragma once
#include "RenderPass.hpp"
#include "PipelineBindings.hpp"

namespace Cobalt
{

	class ForwardPass : public RenderPass
	{
	public:
		ForwardPass();
		~ForwardPass();

	public:
		void SetSkybox(const Cubemap* skybox);

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		void ExecuteSkyboxPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext);
		void ExecuteForwardPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext);

	private:
		PipelineBindings mForwardPipelineBindings, mSkyboxPipelineBindings;
		const Cubemap* mSkybox = nullptr;
		std::vector<std::unique_ptr<VulkanBuffer>> mSkyboxUniformBuffers;

		struct SkyboxUniformBuffer
		{
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
			VkDeviceAddress MeshVertices;
			float __padding0[2];
		};
	};

}
