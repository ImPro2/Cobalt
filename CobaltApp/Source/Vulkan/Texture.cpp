#include "Texture.hpp"
#include "GraphicsContext.hpp"
#include "VulkanBuffer.hpp"

namespace Cobalt
{

	Texture::Texture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels /* = 1 */)
		: mWidth(width), mHeight(height), mFormat(format), mUsage(usage), mMipLevels(mipLevels)
	{
		Recreate(mWidth, mHeight);
	}

	Texture::~Texture()
	{
		Release();
	}

	void Texture::Recreate(uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		Release();

		VkImageCreateInfo imageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = mFormat,
			.extent = { mWidth, mHeight, 1 },
			.mipLevels = mMipLevels,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = mUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		VK_CALL(vkCreateImage(GraphicsContext::Get().GetDevice(), &imageCreateInfo, nullptr, &mImage));

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(GraphicsContext::Get().GetDevice(), mImage, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, GraphicsContext::Get().GetPhysicalDevice())
		};

		VK_CALL(vkAllocateMemory(GraphicsContext::Get().GetDevice(), &memoryAllocateInfo, nullptr, &mMemory));
		VK_CALL(vkBindImageMemory(GraphicsContext::Get().GetDevice(), mImage, mMemory, 0));

		VkImageViewCreateInfo imageViewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.flags = 0,
			.image = mImage,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = mFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VK_CALL(vkCreateImageView(GraphicsContext::Get().GetDevice(), &imageViewCreateInfo, nullptr, &mImageView));

	}

	void Texture::Release()
	{
		if (mImage)
			vkDestroyImage(GraphicsContext::Get().GetDevice(), mImage, nullptr);

		if (mImageView)
			vkDestroyImageView(GraphicsContext::Get().GetDevice(), mImageView, nullptr);

		if (mMemory)
			vkFreeMemory(GraphicsContext::Get().GetDevice(), mMemory, nullptr);
	}

}