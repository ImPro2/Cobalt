#pragma once
#include "Shader.hpp"
#include "VulkanDescriptorSet.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cobalt
{


	struct PipelineInfo
	{
		std::shared_ptr<Shader> Shader;

		VkPrimitiveTopology PrimitiveTopology;

		bool EnableDepthTesting;
	};

	class Pipeline
	{
	public:
		Pipeline(const PipelineInfo& info, VkRenderPass renderPass);
		~Pipeline();

		// Called automatically
		void Invalidate();

		VulkanDescriptorSet* AllocateDescriptorSet(uint32_t set, VkDescriptorPool descriptorPool);

	public:
		VkPipeline GetPipeline() const { return mPipeline; }
		VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
		PipelineInfo mInfo;
		VkRenderPass mRenderPass;

		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;

		std::vector<std::unique_ptr<VulkanDescriptorSet>> mDescriptorSets;
	};

}
