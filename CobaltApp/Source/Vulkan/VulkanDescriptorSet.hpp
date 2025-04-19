#pragma once
#include "VulkanUtils.hpp"
#include "VulkanBuffer.hpp"

namespace Cobalt
{

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(uint32_t setIndex, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkPipelineLayout pipelineLayout);
		~VulkanDescriptorSet();

	public:
		void SetBufferBinding(uint32_t binding, const VulkanBuffer* buffer);
		void Bind(VkCommandBuffer commandBuffer);

	private:
		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

		uint32_t mSetIndex = 0;
	};

}
