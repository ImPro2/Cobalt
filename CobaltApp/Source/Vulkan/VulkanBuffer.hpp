#pragma once
#include <vulkan/vulkan.h>

namespace Cobalt
{

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDeviceHandle);

	class VulkanBuffer
	{
	public:
		VulkanBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags);
		~VulkanBuffer();

	public:
		void Map(VkDeviceSize offset, VkDeviceSize size, void** data);
		void Unmap();

		void CopyData(VkDeviceSize offset, VkDeviceSize size, const void* src);

	public:
		VkBuffer GetBuffer() const { return mBuffer; }
		const VkMemoryRequirements& GetMemoryRequirements() const { return mMemoryRequirements; }
		VkBufferUsageFlags GetUsageFlags() const { return mUsage; }

	private:
		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;
		VkDeviceSize mSize = 0;
		VkBufferUsageFlags mUsage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;

		VkMemoryRequirements mMemoryRequirements;
	};

}