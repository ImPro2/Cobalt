#include "VulkanBuffer.hpp"
#include "GraphicsContext.hpp"
#include "VulkanCommands.hpp"

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

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateGPUBufferFromCPUData(uint32_t offset, uint32_t size, const void* data, VkBufferUsageFlags usage)
	{
		std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Upload data to stagingBuffer

		VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.CopyData(offset, size, data);

		// Copy stagingBuffer to buffer

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::CopyBuffer(commandBuffer, stagingBuffer, *buffer);
		});

		return buffer;
	}

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateMappedBuffer(uint32_t offset, uint32_t size, void** data, VkBufferUsageFlags usage)
	{
		std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		buffer->Map(offset, size, data);

		return buffer;
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