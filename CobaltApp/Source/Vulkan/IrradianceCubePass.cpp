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

		glm::mat4 viewMatrices[6] = {
#if 1
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
#else
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
#endif
		};

		glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f);

		for (uint32_t i = 0; i < 6; i++)
		{
			VSInputData vsInputData{};
			vsInputData.ViewProjection = projection * viewMatrices[i];
			vsInputData.Vertices = mMesh->GetVertexBufferReference();

			mVSInputBuffers[i]->CopyData(&vsInputData);
		}

		FSInputBuffer fsInput;
		fsInput.deltaPhi = (2.0f * glm::pi<float>()) / 180.0f;
		fsInput.deltaTheta = (0.5f * glm::pi<float>()) / 64.0f;

		mFSInputBuffer->CopyData(&fsInput);

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

		mFSInputBuffer = VulkanBuffer::CreateMappedBuffer(sizeof(FSInputBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

		for (uint32_t i = 0; i < 6; i++)
		{
			mVSInputBuffers[i] = VulkanBuffer::CreateMappedBuffer(sizeof(VSInputData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		}
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();
		assert(mEnvironmentMap && mMesh);

		Texture& framebuffer = Renderer::GetRenderGraph().GetResource(mFramebufferHandle);

		VkImageMemoryBarrier2 memoryBarrier0 = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = framebuffer.GetImage(),
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		if (mInvocationIndex > 0)
		{
			VkDependencyInfo dependencyInfo = {
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &memoryBarrier0,
			};

			vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

			framebuffer.SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);

		VkViewport viewport = {
			.width = (float)mDimensions, .height = (float)mDimensions, .minDepth = 0.0f, .maxDepth = 1.0f
		};

		VkRect2D scissor = {
			.extent = { mDimensions, mDimensions }
		};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		ShaderCursor shaderCursor(mPipeline->GetInfo().Shader->GetRootShaderParameter(), mDescriptorHandle);
		shaderCursor.WriteField("vsInput", *(mVSInputBuffers[mInvocationIndex]));
		shaderCursor.WriteField("fsInput", *mFSInputBuffer);
		shaderCursor.Field("fsInput").WriteField("environmentMap", *mEnvironmentMap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipeline->GetPipelineLayout(), mDescriptorHandle);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndices().size(), 1, 0, 0, mInvocationIndex);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);

		VkImageMemoryBarrier2 memoryBarriers[1];
		/*memoryBarriers[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = framebuffer.GetImage(),
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};*/
		memoryBarriers[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = framebuffer.GetImage(),
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		VkDependencyInfo dependencyInfo = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = memoryBarriers,
		};

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

		//VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		framebuffer.SetImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, framebuffer, *mIrradianceCube, mInvocationIndex);
		//VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (mInvocationIndex == 5)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		mInvocationIndex++;
	}

}