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

		mPipeline = GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Irradiance Cube Pipeline", pipelineInfo);

		VkDescriptorSetLayout descriptorSetLayout = pipelineInfo.Shader->GetDescriptorSetLayouts()[0];
		mDescriptorHandle = GraphicsContext::Get().GetDescriptorBufferManager().AllocateDescriptor(descriptorSetLayout, true, true);
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();
		assert(mEnvironmentMap && mMesh);

		uint32_t faceIndex = mInvocationIndex % 6;
		uint32_t mipLevel = mInvocationIndex / 6;

		Texture& framebuffer = Renderer::GetRenderGraph().GetResource(mFramebufferHandle);

		VkImageMemoryBarrier2 memoryBarrier0 = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = /*VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT*/ VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
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

		uint32_t viewportSize = static_cast<float>(mDimensions * std::pow(0.5f, mipLevel));

		VkViewport viewport = {
			.width = (float)viewportSize,
			.height = (float)viewportSize,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		VkRect2D scissor = {
			.extent = { (uint32_t)viewportSize, (uint32_t)viewportSize }
		};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

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

		struct PushConstants
		{
			glm::mat4 ViewProjection;
			VkDeviceAddress Vertices;
			float DeltaTheta = (0.5f * glm::pi<float>()) / 64.0f;
			float DeltaPhi = (2.0f * glm::pi<float>()) / 180.0f;
		};

		PushConstants pushConstants;
		pushConstants.ViewProjection = viewProjection;
		pushConstants.Vertices = mMesh->GetVertexBufferReference();

		//size_t pushConstantBufferSize = mPipeline->GetInfo().Shader->GetPushConstantBufferSize();
		//uint8_t* pushConstantBuffer = new uint8_t[pushConstantBufferSize];
		//std::memset(pushConstantBuffer, 0, pushConstantBufferSize);

		ShaderCursor shaderCursor(mPipeline->GetInfo().Shader->GetRootShaderParameter(), mDescriptorHandle, mPipeline->GetInfo().Shader->GetPushConstantRanges());
		//shaderCursor.WriteField("viewProjection", viewProjection);
		//shaderCursor.WriteField("vertices", mMesh->GetVertexBufferReference());
		//shaderCursor.WriteField("deltaPhi", (2.0f * glm::pi<float>()) / 180.0f);
		//shaderCursor.WriteField("deltaTheta", (0.5f * glm::pi<float>()) / 64.0f);
		shaderCursor.WriteField("environmentMap", *mEnvironmentMap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipeline->GetPipelineLayout(), mDescriptorHandle);
		vkCmdPushConstants(commandBuffer, mPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 80, &pushConstants);
		//vkCmdPushConstants(commandBuffer, mPipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 72, 8, &pushConstantsFragment);
		//vkCmdPushConstants(commandBuffer, mPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 72, 8, &pushConstantsFragment);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndices().size(), 1, 0, 0, 0);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);

		//delete[] pushConstantBuffer;

		VkImageMemoryBarrier2 memoryBarriers[1];
		memoryBarriers[0] = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = /*VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT*/ VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
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
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, framebuffer, *mIrradianceCube, { viewportSize, viewportSize, 1 }, faceIndex, mipLevel);
		//VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (mInvocationIndex == mInvocationCount - 1)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mIrradianceCube, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		mInvocationIndex++;
	}

}