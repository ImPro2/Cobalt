#pragma once
#include "VulkanUtils.hpp"

namespace Cobalt
{

	class Texture
	{
	public:
		Texture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = 1);
		~Texture();

		void Recreate(uint32_t width, uint32_t height);

	private:
		void Release();

	public:
		VkImage GetImage() const { return mImage; }
		VkImageView GetImageView() const { return mImageView; }
		VkDeviceMemory GetMemory() const { return mMemory; }

		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }

		VkFormat GetFormat() const { return mFormat; }

	private:
		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;

		uint32_t mWidth, mHeight;

		VkFormat mFormat;
		VkImageUsageFlags mUsage;
		uint32_t mMipLevels;
	};

}
