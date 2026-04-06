#include "copch.hpp"
#include "IrradianceCubePass.hpp"
#include "GraphicsContext.hpp"
#include "VulkanCommands.hpp"
#include "Renderer.hpp"
#include "RenderGraph.hpp"

#include <cmath>

namespace Cobalt
{

	IrradianceCubePass::IrradianceCubePass()
		: CubemapPass("Irradiance Cube Pass", "", (RenderPassFlags)RenderPassFlagBits::SideAffect, 32, VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		CO_PROFILE_FN();
	}

	IrradianceCubePass::~IrradianceCubePass()
	{
		CO_PROFILE_FN();
	}

	void IrradianceCubePass::SetEnvironmentMap(Cubemap* envMap)
	{
		CO_PROFILE_FN();

		mEnvironmentMap = envMap;
		mMesh = envMap->GetMesh();
		mCubemap->SetMesh(mMesh);
	}

	void IrradianceCubePass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		CubemapPass::Setup(builder);

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\IrradianceCube.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, mFormat }
			}
		};

		Pipeline* pipeline = GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Irradiance Cube Pipeline", pipelineInfo);
		mPipelineBindings = PipelineBindings(pipeline, false);
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize)
	{
		CO_PROFILE_FN();

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, { viewportSize, viewportSize }, false);

		static glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);
		glm::mat4 viewProjection = projection * mViewMatrices[face];

		ShaderCursor shaderCursor = mPipelineBindings.GetShaderCursor();
		shaderCursor.Field("uniforms")
			.WriteField("ViewProjection", viewProjection)
			.WriteField("Vertices", mMesh->GetVertexBufferReference())
			.WriteField("DeltaPhi", (2.0f * glm::pi<float>()) / 180.0f)
			.WriteField("DeltaTheta", (0.5f * glm::pi<float>()) / 64.0f);
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