#pragma once
#include "VulkanUtils.hpp"
#include "VulkanBuffer.hpp"
#include "Texture.hpp"

namespace Cobalt
{

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(uint32_t setIndex, VkDescriptorSet descriptorSet, VkPipelineLayout pipelineLayout);
		~VulkanDescriptorSet();

	public:
		void SetBufferBinding(const VulkanBuffer* buffer, uint32_t binding, uint32_t arrayIndex = 0);
		void SetImageBinding(const Texture* image, uint32_t binding, uint32_t arrayIndex = 0);
		void Bind(VkCommandBuffer commandBuffer);

	public:
		VkDescriptorSet GetDescriptorSet() const { return mDescriptorSet; }
		VkDescriptorSet* GetDescriptorSetPtr() { return &mDescriptorSet; }

	private:
		uint32_t mSetIndex = 0;

		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

	};

}
