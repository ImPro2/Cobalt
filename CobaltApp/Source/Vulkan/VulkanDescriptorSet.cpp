#include "VulkanDescriptorSet.hpp"
#include "GraphicsContext.hpp"

namespace Cobalt
{

	VulkanDescriptorSet::VulkanDescriptorSet(uint32_t setIndex, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkPipelineLayout pipelineLayout)
		: mDescriptorSetLayout(descriptorSetLayout), mDescriptorPool(descriptorPool), mPipelineLayout(pipelineLayout), mSetIndex(setIndex)
	{
		VkDescriptorSetAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = mDescriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &mDescriptorSetLayout
		};

		VK_CALL(vkAllocateDescriptorSets(GraphicsContext::Get().GetDevice(), &allocInfo, &mDescriptorSet));
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		vkFreeDescriptorSets(GraphicsContext::Get().GetDevice(), mDescriptorPool, 1, &mDescriptorSet);
	}

	void VulkanDescriptorSet::SetBufferBinding(uint32_t binding, const VulkanBuffer* buffer)
	{
		VkDescriptorBufferInfo descriptorBufferInfo = {
			.buffer = buffer->GetBuffer(),
			.offset = 0,
			.range = buffer->GetMemoryRequirements().size
		};

		VkDescriptorType descriptorType;

		switch (buffer->GetUsageFlags())
		{
			case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
			case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
		}

		VkWriteDescriptorSet writeDescSet = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = mDescriptorSet,
			.dstBinding = binding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = descriptorType,
			.pBufferInfo = &descriptorBufferInfo,
		};

		VkWriteDescriptorSet writeDescSets[] = { writeDescSet };

		vkUpdateDescriptorSets(GraphicsContext::Get().GetDevice(), sizeof(writeDescSets) / sizeof(writeDescSets[0]), writeDescSets, 0, nullptr);
	}

	void VulkanDescriptorSet::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, mSetIndex, 1, &mDescriptorSet, 0, nullptr);
	}

}