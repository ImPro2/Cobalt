#pragma once
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Pipeline.hpp"
#include "ShaderCursor.hpp"
#include "Texture.hpp"
#include "PipelineBindings.hpp"

#include <vector>

namespace Cobalt
{

	class LightingPass : public RenderPass
	{
	public:
		LightingPass();
		~LightingPass();

	public:
		void SetSkybox(const Cubemap* skybox);

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		void ExecuteSkyboxPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext);
		void ExecuteLightingPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext);

	private:
		RGResourceHandle mPositionAttachment, mBaseColorAttachment, mNormalAttachment, mOCRAttachment, mEmissiveAttachment;
		PipelineBindings mSkyboxPipelineBindings, mLightingPipelineBindings;

		struct SkyboxUniformBuffer
		{
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
			VkDeviceAddress MeshVertices;
			float __padding0[2];
		};

		std::vector<std::unique_ptr<VulkanBuffer>> mSkyboxUniformBuffers;
		const Cubemap* mSkybox = nullptr;
	};

}
