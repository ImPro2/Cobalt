#include "copch.hpp"
#include "PrefilteredEnvironmentMapPass.hpp"
#include "RenderGraph.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	PrefilteredEnvironmentMapPass::PrefilteredEnvironmentMapPass()
		: CubemapPass("Prefiltered Environment Map Pass", "", (RenderPassFlags)RenderPassFlagBits::None, 512, VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		CO_PROFILE_FN();
	}

	PrefilteredEnvironmentMapPass::~PrefilteredEnvironmentMapPass()
	{
		CO_PROFILE_FN();
	}

	void PrefilteredEnvironmentMapPass::SetEnvironmentMap(const Cubemap* environmentMap, const Mesh* mesh)
	{
		CO_PROFILE_FN();

		mEnvironmentMap = environmentMap;
		mMesh = mesh;
	}

	void PrefilteredEnvironmentMapPass::Setup(RenderGraphBuilder& builder)
	{
		CubemapPass::Setup(builder);

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\PrefilteredEnvironmentMap.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, mFormat }
			}
		};

		mPipelineBindings = PipelineBindings(GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("PrefilteredEnvironmentMap", pipelineInfo), false);
	}

	void PrefilteredEnvironmentMapPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize)
	{
		CO_PROFILE_FN();

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, { viewportSize, viewportSize }, false);

		static glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);
		glm::mat4 viewProjection = projection * mViewMatrices[face];

		float roughness = (float)mipLevel / (mMipLevels - 1);

		ShaderCursor shaderCursor = mPipelineBindings.GetShaderCursor();
		shaderCursor.Field("uniforms")
			.WriteField("ViewProjection", viewProjection)
			.WriteField("Vertices", mMesh->GetVertexBufferReference())
			.WriteField("Roughness", roughness)
			.WriteField("SampleCount", 32);
		shaderCursor.WriteField("environmentMap", *mEnvironmentMap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipelineBindings.PipelineRef->GetPipelineLayout(), mPipelineBindings.DescriptorHandles[0]);
		vkCmdPushConstants(commandBuffer, mPipelineBindings.PipelineRef->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, mPipelineBindings.PushConstantBuffer.size(), mPipelineBindings.PushConstantBuffer.data());
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineBindings.PipelineRef->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndices().size(), 1, 0, 0, 0);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);
	}

}
