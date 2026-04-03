#include "copch.hpp"
#include "CubemapPass.hpp"
#include "RenderGraph.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	CubemapPass::CubemapPass(const std::string& passName, const std::filesystem::path& shaderPath, RenderPassFlags flags, uint32_t dimensions, VkFormat format)
		: RenderPass(passName, shaderPath, flags), mDimensions(dimensions), mFormat(format)
	{
		CO_PROFILE_FN();

		mMipLevels = static_cast<uint32_t>(floor(log2(mDimensions))) + 1;
		mInvocationCount = 6 * mMipLevels;

		mViewMatrices = {
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
	}

	CubemapPass::~CubemapPass()
	{
		CO_PROFILE_FN();
	}

	void CubemapPass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mFramebufferHandle = builder.DeclareResource("Irradiance Cube Framebuffer", RGResourceInfo{
			.ResourceType = RGResourceType::ColorAttachment,
			.ResourceSizeFlags = RGResourceSizeFlags::Absolute,
			.Format = mFormat,
			.Transient = false,
			.CopySrc = true,
			.Width = mDimensions,
			.Height = mDimensions
		});

		builder.AddDependency(mFramebufferHandle, RGAccessType::ColorAttachmentWrite);
		builder.SetClearColor(mFramebufferHandle, { 0.0f, 0.0f, 0.2f, 0.0f });
		builder.SetExecutionCount(mInvocationCount);

		mCubemap = std::make_unique<Cubemap>(CubemapInfo {
			.Format = mFormat,
			.Width = mDimensions,
			.Height = mDimensions,
			.MipLevels = mMipLevels
		});

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mCubemap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		});
	}

	void CubemapPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		uint32_t face = mInvocationIndex % 6;
		uint32_t mipLevel = mInvocationIndex / 6;
		uint32_t viewportSize = static_cast<float>(mDimensions * std::pow(0.5f, mipLevel));

		Execute(commandBuffer, renderContext, face, mipLevel, viewportSize);

		// Copy framebuffer to cubemap face and mip level

		Texture& framebuffer = *mRenderGraph.GetResource(mFramebufferHandle);

		framebuffer.SetImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, framebuffer, *mCubemap, { viewportSize, viewportSize, 1 }, face, mipLevel);
		VulkanCommands::TransitionImageLayout(commandBuffer, framebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		if (mInvocationIndex == mInvocationCount - 1)
			VulkanCommands::TransitionImageLayout(commandBuffer, *mCubemap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		mInvocationIndex++;
	}

}