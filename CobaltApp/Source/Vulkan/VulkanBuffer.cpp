#include "copch.hpp"
#include "VulkanBuffer.hpp"
#include "GraphicsContext.hpp"
#include "VulkanCommands.hpp"


namespace Cobalt
{

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDeviceHandle)
	{
		CO_PROFILE_FN();

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

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateGPUBufferFromCPUData(const void* data, uint32_t size, VkBufferUsageFlags usage)
	{
		CO_PROFILE_FN();

		//std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Upload data to stagingBuffer

		//VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//stagingBuffer.CopyData(offset, size, data);

		std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

		std::unique_ptr<VulkanBuffer> stagingBuffer = CreateMappedBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingBuffer->CopyData(data);

		// Copy stagingBuffer to buffer

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::CopyBuffer(commandBuffer, *stagingBuffer, *buffer);
		});

		return buffer;
	}

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateMappedBuffer(uint32_t size, VkBufferUsageFlags usage)
	{
		CO_PROFILE_FN();

		//std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//buffer->Map(offset, size, data);

		std::unique_ptr<VulkanBuffer> buffer = std::make_unique<VulkanBuffer>(size, usage, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

		return buffer;
	}

	VulkanBuffer::VulkanBuffer(uint32_t size, VkBufferUsageFlags usage, VmaAllocationCreateFlags allocationFlags)
		: mUsage(usage)
	{
		CO_PROFILE_FN();

		VkBufferCreateInfo bufferCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.flags = 0,
			.size = size,
			.usage = mUsage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo allocCreateInfo = {
			.flags = allocationFlags,
			.usage = VMA_MEMORY_USAGE_AUTO,
		};

		//VK_CALL(vkCreateBuffer(GraphicsContext::Get().GetDevice(), &bufferCreateInfo, nullptr, &mBuffer));
		VK_CALL(vmaCreateBuffer(GraphicsContext::Get().GetAllocator(), &bufferCreateInfo, &allocCreateInfo, &mBuffer, &mAllocation, &mAllocationInfo));

#if 0
		vkGetBufferMemoryRequirements(GraphicsContext::Get().GetDevice(), mBuffer, &mMemoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = mMemoryRequirements.size,
			.memoryTypeIndex = FindMemoryType(mMemoryRequirements.memoryTypeBits, memoryPropertyFlags, GraphicsContext::Get().GetPhysicalDevice())
		};

		VK_CALL(vkAllocateMemory(GraphicsContext::Get().GetDevice(), &memoryAllocateInfo, nullptr, &mMemory));

		VK_CALL(vkBindBufferMemory(GraphicsContext::Get().GetDevice(), mBuffer, mMemory, 0));
#endif
	}

	VulkanBuffer::~VulkanBuffer()
	{
		CO_PROFILE_FN();

		//vkFreeMemory(GraphicsContext::Get().GetDevice(), mMemory, nullptr);
		//vkDestroyBuffer(GraphicsContext::Get().GetDevice(), mBuffer, nullptr);
		vmaDestroyBuffer(GraphicsContext::Get().GetAllocator(), mBuffer, mAllocation);
	}

	void VulkanBuffer::Map(void** data)
	{
		CO_PROFILE_FN();

		//VK_CALL(vkMapMemory(GraphicsContext::Get().GetDevice(), mMemory, offset, size, 0, data));
		VK_CALL(vmaMapMemory(GraphicsContext::Get().GetAllocator(), mAllocation, data));
	}

	void VulkanBuffer::Unmap()
	{
		CO_PROFILE_FN();

		//vkUnmapMemory(GraphicsContext::Get().GetDevice(), mMemory);
		vmaUnmapMemory(GraphicsContext::Get().GetAllocator(), mAllocation);
	}

	void VulkanBuffer::CopyData(const void* src, uint32_t size)
	{
		CO_PROFILE_FN();

		memcpy(mAllocationInfo.pMappedData, src, size == 0 ? mAllocationInfo.size : size);
	}

	VkDeviceAddress VulkanBuffer::GetDeviceAddress() const
	{
		CO_PROFILE_FN();

		VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = mBuffer
		};

		return vkGetBufferDeviceAddress(GraphicsContext::Get().GetDevice(), &bufferDeviceAddressInfo);
	}

}