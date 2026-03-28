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

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		});
	}

	void IrradianceCubePass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mFramebufferHandle = builder.DeclareResource("Irradiance Cube Framebuffer", RGResourceInfo {
			.ResourceType = RGResourceType::ColorAttachment,
			.ResourceSizeFlags = RGResourceSizeFlags::Absolute,
			.Transient = false,
			.CopySrc = true,
			.Width = mDimensions,
			.Height = mDimensions
		});

		builder.AddDependency(mFramebufferHandle, RGAccessType::ColorAttachmentWrite);
		builder.SetClearColor(mFramebufferHandle);
		builder.SetExecutionCount(6);

		CubemapInfo irradianceCubeInfo = {
			.Format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.Width = mDimensions,
			.Height = mDimensions,
		};

		mIrradianceCube = std::make_unique<Cubemap>(irradianceCubeInfo);

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\IrradianceCube.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, VK_FORMAT_R32G32B32A32_SFLOAT }
			}
		};

		mPipeline = GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Irradiance Cube Pipeline", pipelineInfo);

		VkDescriptorSetLayout descriptorSetLayout = pipelineInfo.Shader->GetDescriptorSetLayouts()[0];
		mDescriptorHandle = GraphicsContext::Get().GetDescriptorBufferManager().AllocateDescriptor(descriptorSetLayout, true, true);

		mVSInputBuffer = VulkanBuffer::CreateMappedBuffer(sizeof(VSInputBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		mFSInputBuffer = VulkanBuffer::CreateMappedBuffer(sizeof(FSInputBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();
		assert(mEnvironmentMap && mMesh);

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		//VulkanCommands::SetViewport(commandBuffer, { mDimensions, mDimensions });

		VkViewport viewport = {
			.width = (float)mDimensions, .height = (float)mDimensions, .minDepth = 0.0f, .maxDepth = 1.0f
		};

		VkRect2D scissor = {
			.extent = { mDimensions, mDimensions }
		};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		static uint32_t invocationIndex = 0;

		static glm::mat4 viewMatrices[6] = {
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

		static glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);

		VSInputBuffer vsInput;
		vsInput.ViewProjections[invocationIndex] = projection * viewMatrices[invocationIndex];
		vsInput.Vertices = mMesh->GetVertexBufferReference();
		vsInput.InvocationIndex = invocationIndex;

		FSInputBuffer fsInput;
		fsInput.deltaPhi = (2.0f * glm::pi<float>()) / 180.0f;
		fsInput.deltaTheta = (0.5f * glm::pi<float>()) / 64.0f;

		mVSInputBuffer->CopyData(&vsInput);
		mFSInputBuffer->CopyData(&fsInput);

		//VulkanCommands::SetViewport(commandBuffer, { mEnvironmentMap->GetWidth(), mEnvironmentMap->GetHeight() });

		ShaderCursor shaderCursor(mPipeline->GetInfo().Shader->GetRootShaderParameter(), mDescriptorHandle);
		shaderCursor.WriteField("vsInput", *mVSInputBuffer);
		shaderCursor.WriteField("fsInput", *mFSInputBuffer);
		shaderCursor.Field("fsInput").WriteField("environmentMap", *mEnvironmentMap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipeline->GetPipelineLayout(), mDescriptorHandle);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndices().size(), 1, 0, 0, 0);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);

		Texture& framebuffer = Renderer::GetRenderGraph().GetResource(mFramebufferHandle);

		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, framebuffer, *mIrradianceCube, invocationIndex);
		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (invocationIndex == 5)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		invocationIndex++;
	}

}