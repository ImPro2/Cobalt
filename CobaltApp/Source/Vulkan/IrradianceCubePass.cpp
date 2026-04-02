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
		: RenderPass("Irradiance Cube Pass", "", (RenderPassFlags)RenderPassFlagBits::SideAffect)
	{
		CO_PROFILE_FN();
	}

	IrradianceCubePass::~IrradianceCubePass()
	{
		CO_PROFILE_FN();
	}

	void IrradianceCubePass::SetEnvironmentMap(Cubemap* envMap, const Mesh* mesh)
	{
		CO_PROFILE_FN();

		mEnvironmentMap = envMap;
		mMesh = mesh;
	}

	void IrradianceCubePass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mFramebufferHandle = builder.DeclareResource("Irradiance Cube Framebuffer", RGResourceInfo{
			.ResourceType = RGResourceType::ColorAttachment,
			.ResourceSizeFlags = RGResourceSizeFlags::Absolute,
			.Transient = false,
			.CopySrc = true,
			.Width = mDimensions,
			.Height = mDimensions
		});

		builder.AddDependency(mFramebufferHandle, RGAccessType::ColorAttachmentWrite);
		builder.SetClearColor(mFramebufferHandle, { 0.0f, 0.0f, 0.2f, 0.0f });

		CubemapInfo irradianceCubeInfo = {
			.Format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.Width = mDimensions,
			.Height = mDimensions,
			.MipLevels = static_cast<uint32_t>(floor(log2(mDimensions))) + 1
		};

		mInvocationCount = 6 * irradianceCubeInfo.MipLevels;
		builder.SetExecutionCount(mInvocationCount);

		mIrradianceCube = std::make_unique<Cubemap>(irradianceCubeInfo);

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		});

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\IrradianceCube.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, VK_FORMAT_R32G32B32A32_SFLOAT }
			}
		};

		Pipeline* pipeline = GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Irradiance Cube Pipeline", pipelineInfo);
		mPipelineBindings = PipelineBindings(pipeline, false);
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();
		assert(mEnvironmentMap && mMesh);

		uint32_t faceIndex = mInvocationIndex % 6;
		uint32_t mipLevel = mInvocationIndex / 6;
		uint32_t viewportSize = static_cast<float>(mDimensions * std::pow(0.5f, mipLevel));

		Texture& framebuffer = mRenderGraph.GetResource(mFramebufferHandle);

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, { viewportSize, viewportSize }, false);

		glm::mat4 viewMatrices[6] = {
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);
		glm::mat4 viewProjection = projection * viewMatrices[faceIndex];

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

		framebuffer.SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, framebuffer, *mIrradianceCube, { viewportSize, viewportSize, 1 }, faceIndex, mipLevel);
		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (mInvocationIndex == mInvocationCount - 1)
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		mInvocationIndex++;
	}

}