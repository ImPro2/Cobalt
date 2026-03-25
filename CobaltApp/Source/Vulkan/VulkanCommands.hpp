#pragma once
#include "OptickMacros.hpp"
#include "VulkanUtils.hpp"
#include "GraphicsContext.hpp"
#include "VulkanBuffer.hpp"
#include "Texture.hpp"

namespace Cobalt
{

	class VulkanCommands
	{
	public:
		static void SetViewport(VkCommandBuffer commandBuffer, VkExtent2D extent)
		{
			CO_PROFILE_FN();

			VkViewport viewport = {
				.x = 0,
				.y = (float)extent.height,
				.width = (float)extent.width,
				.height = -(float)extent.height,
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			VkRect2D scissor = {
				.extent = extent
			};

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		}

		static void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0)
		{
			CO_PROFILE_FN();

			VkBufferCopy bufferCopy = {
				.srcOffset = srcOffset,
				.dstOffset = dstOffset,
				.size = size
			};

			vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);
		}

		static void CopyBuffer(VkCommandBuffer commandBuffer, const VulkanBuffer& srcBuffer, const VulkanBuffer& dstBuffer, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0)
		{
			CO_PROFILE_FN();

			CopyBuffer(commandBuffer, srcBuffer.GetBuffer(), dstBuffer.GetBuffer(), srcBuffer.GetAllocationInfo().size, srcOffset, dstOffset);
		}

		// Image layout has to be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		static void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, VkImageAspectFlags imageAspect, VkExtent3D imageExtent, VkDeviceSize bufferOffset = 0, VkOffset3D imageOffset = { 0, 0, 0 })
		{
			CO_PROFILE_FN();

			VkBufferImageCopy bufferImageCopy = {
				.bufferOffset = bufferOffset,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = imageAspect,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.imageOffset = imageOffset,
				.imageExtent = imageExtent
			};

			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
		}

		static void CopyBufferToImage(VkCommandBuffer commandBuffer, const VulkanBuffer& buffer, const Texture& image, VkDeviceSize bufferOffset = 0, VkOffset3D imageOffset = { 0, 0, 0 })
		{
			CO_PROFILE_FN();

			CopyBufferToImage(commandBuffer, buffer.GetBuffer(), image.GetImage(), image.GetImageAspectFlags(), { image.GetWidth(), image.GetHeight(), 1 }, bufferOffset, imageOffset);
		}

		static void CopyImage(VkCommandBuffer commandBuffer, uint32_t srcBaseLayer, uint32_t dstBaseLayer, VkExtent3D extent, VkImage srcImage, VkImage dstImage, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout)
		{
			CO_PROFILE_FN();

			VkImageCopy copyRegion = {
				.srcSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseArrayLayer = srcBaseLayer,
				},
				.dstSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseArrayLayer = dstBaseLayer,
				},
				.extent = extent
			};

			vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1, &copyRegion);
		}

		static void CopyImageToCubemapFace(VkCommandBuffer commandBuffer, const Texture& texture, const Cubemap& cubemap, uint32_t face)
		{
			CO_PROFILE_FN();

			CopyImage(commandBuffer, 0, face, { texture.GetWidth(), texture.GetHeight(), 1 }, texture.GetImage(), cubemap.GetImage(), texture.GetImageLayout(), cubemap.GetImageLayout());
		}

		static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags imageAspect, uint32_t mipLevels, uint32_t layers, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
		{
			CO_PROFILE_FN();

			auto [srcAccess, srcStage] = GetSyncOptsFromImageLayout(oldImageLayout);
			auto [dstAccess, dstStage] = GetSyncOptsFromImageLayout(newImageLayout);

			VkImageMemoryBarrier imageMemoryBarrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = srcAccess,
				.dstAccessMask = dstAccess,
				.oldLayout = oldImageLayout,
				.newLayout = newImageLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = image,
				.subresourceRange = {
					.aspectMask = imageAspect,
					.baseMipLevel = 0,
					.levelCount = mipLevels,
					.baseArrayLayer = 0,
					.layerCount = layers
				}
			};

			vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		static void TransitionImageLayout(VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newImageLayout)
		{
			CO_PROFILE_FN();

			VkImageLayout oldImageLayout = texture.GetImageLayout();

			TransitionImageLayout(commandBuffer, texture.GetImage(), texture.GetImageAspectFlags(), texture.GetMipMapLevels(), 1, oldImageLayout, newImageLayout);

			texture.SetImageLayout(newImageLayout);
		}

		static void TransitionImageLayout(VkCommandBuffer commandBuffer, Cubemap& cubemap, VkImageLayout newImageLayout)
		{
			CO_PROFILE_FN();

			VkImageLayout oldImageLayout = cubemap.GetImageLayout();

			TransitionImageLayout(commandBuffer, cubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, cubemap.GetMipMapLevels(), 6, oldImageLayout, newImageLayout);

			cubemap.SetImageLayout(newImageLayout);
		}

	private:
		static std::pair<VkAccessFlags, VkPipelineStageFlags> GetSyncOptsFromImageLayout(VkImageLayout imageLayout)
		{
			CO_PROFILE_FN();

			switch (imageLayout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:                return { 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:     return { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
			}

			return {};
		}
	};

}
