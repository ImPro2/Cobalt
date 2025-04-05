#include "VulkanBuffer.hpp"
#include "GraphicsContext.hpp"

namespace Cobalt
{

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDeviceHandle)
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDeviceHandle, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		return 0;
	}

	VulkanBuffer::VulkanBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags)
		: mSize(size), mUsage(usage)
	{
		VkBufferCreateInfo bufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.flags = 0,
			.size = mSize,
			.usage = mUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VK_CALL(vkCreateBuffer(GraphicsContext::Get().GetDevice(), &bufferCreateInfo, nullptr, &mBuffer));

		vkGetBufferMemoryRequirements(GraphicsContext::Get().GetDevice(), mBuffer, &mMemoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mMemoryRequirements.size,
			.memoryTypeIndex = FindMemoryType(mMemoryRequirements.memoryTypeBits, memoryPropertyFlags, GraphicsContext::Get().GetPhysicalDevice())
		};

		VK_CALL(vkAllocateMemory(GraphicsContext::Get().GetDevice(), &memoryAllocateInfo, nullptr, &mMemory));

		VK_CALL(vkBindBufferMemory(GraphicsContext::Get().GetDevice(), mBuffer, mMemory, 0));
	}

	VulkanBuffer::~VulkanBuffer()
	{
		vkFreeMemory(GraphicsContext::Get().GetDevice(), mMemory, nullptr);
		vkDestroyBuffer(GraphicsContext::Get().GetDevice(), mBuffer, nullptr);
	}

	void VulkanBuffer::Map(VkDeviceSize offset, VkDeviceSize size, void** data)
	{
		VK_CALL(vkMapMemory(GraphicsContext::Get().GetDevice(), mMemory, offset, size, 0, data));
	}

	void VulkanBuffer::Unmap()
	{
		vkUnmapMemory(GraphicsContext::Get().GetDevice(), mMemory);
	}

	void VulkanBuffer::CopyData(VkDeviceSize offset, VkDeviceSize size, const void* src)
	{
		void* data;

		Map(offset, size, &data);
		std::memcpy(data, src, size);
		Unmap();
	}

}