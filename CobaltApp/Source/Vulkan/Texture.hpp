#pragma once
#include "VulkanUtils.hpp"
#include <string>

namespace Cobalt
{

	class Texture
	{
	public:
		static std::unique_ptr<Texture> CreateFromFile(const std::string& filePath);

	public:
		Texture(const std::string& filePath);
		Texture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = 1);
		~Texture();

		void Recreate(uint32_t width, uint32_t height);

	private:
		uint8_t* LoadDataFromFile(const std::string& filePath);
		void Release();

	public:
		VkImage GetImage() const { return mImage; }
		VkImageView GetImageView() const { return mImageView; }
		VkDeviceMemory GetMemory() const { return mMemory; }
		VkSampler GetSampler() const { return mSampler; }

		VkImageLayout GetImageLayout() const { return mImageLayout; }
		void SetImageLayout(VkImageLayout layout) { mImageLayout = layout; }

		VkImageAspectFlags GetImageAspectFlags() const { return mImageAspect; }

		uint32_t GetWidth() const { return mWidth; }
		uint32_t GetHeight() const { return mHeight; }

		VkFormat GetFormat() const { return mFormat; }

		uint32_t GetMipMapLevels() const { return mMipLevels; }

	private:
		VkImage mImage = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;

		VkImageLayout mImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		uint32_t mWidth, mHeight;
		uint32_t mImageSize;

		VkFormat mFormat;
		VkImageUsageFlags mUsage;
		VkImageAspectFlags mImageAspect;
		uint32_t mMipLevels;
	};

}
