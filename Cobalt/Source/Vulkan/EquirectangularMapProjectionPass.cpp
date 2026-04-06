#include "copch.hpp"
#include "EquirectangularMapProjectionPass.hpp"
#include "RenderGraph.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	EquirectangularProjectionPass::EquirectangularProjectionPass()
		: CubemapPass("Equirectangular Projection Pass", "", (RenderPassFlags)RenderPassFlagBits::None, 512, VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		CO_PROFILE_FN();
	}

	EquirectangularProjectionPass::~EquirectangularProjectionPass()
	{
		CO_PROFILE_FN();
	}

	void EquirectangularProjectionPass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		CubemapPass::Setup(builder);

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("Common\\EquirectangularMapProjection.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, mFormat }
			}
		};

		mPipelineBindings = PipelineBindings(GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Equirectangular Map Projection", pipelineInfo), false);
	}

	void EquirectangularProjectionPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize)
	{
		CO_PROFILE_FN();

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, { viewportSize, viewportSize }, false);

		static glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);
		glm::mat4 viewProjection = projection * mViewMatrices[face];

		ShaderCursor shaderCursor = mPipelineBindings.GetShaderCursor();
		shaderCursor.Field("uniforms")
			.WriteField("ViewProjection", viewProjection)
			.WriteField("Vertices", mMesh->GetVertexBufferReference());
		shaderCursor.WriteField("equirectangularMap", *mEquirectangularMap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipelineBindings.PipelineRef->GetPipelineLayout(), mPipelineBindings.DescriptorHandles[0]);
		vkCmdPushConstants(commandBuffer, mPipelineBindings.PipelineRef->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, mPipelineBindings.PushConstantBuffer.size(), mPipelineBindings.PushConstantBuffer.data());
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineBindings.PipelineRef->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndices().size(), 1, 0, 0, 0);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);
	}

}