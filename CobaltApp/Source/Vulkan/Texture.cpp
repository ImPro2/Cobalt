#include "Texture.hpp"
#include "GraphicsContext.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCommands.hpp"

#include <stb_image.h>

namespace Cobalt
{

	Texture::Texture(const TextureInfo& textureInfo)
		: mWidth(textureInfo.Width), mHeight(textureInfo.Height), mFormat(textureInfo.Format), mUsage(textureInfo.Usage), mMipLevels(textureInfo.MipLevels)
	{
		if (textureInfo.LoadFromFile())
		{
			uint8_t* data = LoadDataFromFile(textureInfo.FilePath);
			Recreate(mWidth, mHeight);

			//VulkanBuffer stagingBuffer(mImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			std::unique_ptr<VulkanBuffer> stagingBuffer = VulkanBuffer::CreateMappedBuffer(mImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			stagingBuffer->CopyData(data);

			GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
			{
				VulkanCommands::TransitionImageLayout(commandBuffer, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				VulkanCommands::CopyBufferToImage(commandBuffer, *stagingBuffer, *this);
				VulkanCommands::TransitionImageLayout(commandBuffer, *this, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			});
		}
		else
		{
			Recreate(mWidth, mHeight);
		}
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

		// Create image

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

		VmaAllocationCreateInfo allocCreateInfo = {
			.usage = VMA_MEMORY_USAGE_AUTO,
		};

		//VK_CALL(vkCreateImage(GraphicsContext::Get().GetDevice(), &imageCreateInfo, nullptr, &mImage));
		VK_CALL(vmaCreateImage(GraphicsContext::Get().GetAllocator(), &imageCreateInfo, &allocCreateInfo, &mImage, &mAllocation, &mAllocationInfo));

#if 0
		// Allocate memory

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(GraphicsContext::Get().GetDevice(), mImage, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, GraphicsContext::Get().GetPhysicalDevice())
		};

		VK_CALL(vkAllocateMemory(GraphicsContext::Get().GetDevice(), &memoryAllocateInfo, nullptr, &mMemory));
		VK_CALL(vkBindImageMemory(GraphicsContext::Get().GetDevice(), mImage, mMemory, 0));
#endif

		// Create image view

		if (mFormat == VK_FORMAT_D16_UNORM_S8_UINT || mFormat == VK_FORMAT_D16_UNORM)
			mImageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		else
			mImageAspect = VK_IMAGE_ASPECT_COLOR_BIT;

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
				.aspectMask = mImageAspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VK_CALL(vkCreateImageView(GraphicsContext::Get().GetDevice(), &imageViewCreateInfo, nullptr, &mImageView));

		// Create sampler if needed

		if (mUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(GraphicsContext::Get().GetPhysicalDevice(), &physicalDeviceProperties);

			VkSamplerCreateInfo samplerCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.flags = 0,
				.magFilter = VK_FILTER_LINEAR,
				.minFilter = VK_FILTER_LINEAR,
				.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
				.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.mipLodBias = 0.0f,
				.anisotropyEnable = VK_TRUE,
				.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy,
				.compareEnable = VK_FALSE,
				.compareOp = {},
				.minLod = 0.0f,
				.maxLod = VK_LOD_CLAMP_NONE,
				.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
				.unnormalizedCoordinates = VK_FALSE,
			};

			VK_CALL(vkCreateSampler(GraphicsContext::Get().GetDevice(), &samplerCreateInfo, nullptr, &mSampler));
		}
	}

	uint8_t* Texture::LoadDataFromFile(const std::string& filePath)
	{
		int32_t width, height, channels;
		stbi_uc* data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!data || channels != 4)
		{
			std::cerr << "Failed to load texture" << filePath << std::endl;
		}

		mWidth = width;
		mHeight = height;
		mImageSize = mWidth * mHeight * channels;
		mFormat = VK_FORMAT_R8G8B8A8_SRGB;
		mUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		//mMipLevels = (uint32_t)std::floor(std::log2(std::max(mWidth, mHeight))) + 1;
		mMipLevels = 1;

		// TODO: mipmaps

		return data;
	}

	void Texture::Release()
	{
		//if (mImage)
			//vkDestroyImage(GraphicsContext::Get().GetDevice(), mImage, nullptr);

		vmaDestroyImage(GraphicsContext::Get().GetAllocator(), mImage, mAllocation);

		if (mImageView)
			vkDestroyImageView(GraphicsContext::Get().GetDevice(), mImageView, nullptr);

		//if (mMemory)
			//vkFreeMemory(GraphicsContext::Get().GetDevice(), mMemory, nullptr);
	}

}