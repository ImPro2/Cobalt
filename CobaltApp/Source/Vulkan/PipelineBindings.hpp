#pragma once
#include "VulkanUtils.hpp"
#include "Pipeline.hpp"
#include "GraphicsContext.hpp"
#include "DescriptorBufferManager.hpp"
#include "ShaderCursor.hpp"

namespace Cobalt
{

	struct PipelineBindings
	{
		PipelineBindings() = default;
		PipelineBindings(Pipeline* pipeline, bool perFrameDescriptors = true)
			: PipelineRef(pipeline)
		{
			AllocateDescriptors(perFrameDescriptors);
			AllocatePushConstantBuffer();
		}

		void AllocateDescriptors(bool perFrameDescriptors = true)
		{
			CO_PROFILE_FN();

			auto& descriptorBufferManager = GraphicsContext::Get().GetDescriptorBufferManager();
			VkDescriptorSetLayout descriptorSetLayout = PipelineRef->GetInfo().Shader->GetDescriptorSetLayouts()[0];

			uint32_t descriptorCount = perFrameDescriptors ? GraphicsContext::Get().GetFrameCount() : 1;
			DescriptorHandles.resize(descriptorCount);

			for (uint32_t i = 0; i < descriptorCount; i++)
			{
				DescriptorHandles[i] = descriptorBufferManager.AllocateDescriptor(descriptorSetLayout, true, true);
			}
		}

		void AllocatePushConstantBuffer()
		{
			CO_PROFILE_FN();

			size_t size = PipelineRef->GetInfo().Shader->GetPushConstantBufferSize();
			PushConstantBuffer.resize(size);
		}

		ShaderCursor GetShaderCursor(uint32_t frameIndex = 0)
		{
			CO_PROFILE_FN();

			Shader* shader = PipelineRef->GetInfo().Shader;

			return ShaderCursor(
				shader->GetRootShaderParameter(),
				DescriptorHandles[frameIndex],
				shader->GetPushConstantRanges(),
				PushConstantBuffer.empty() ? nullptr : PushConstantBuffer.data()
			);
		}

		Pipeline* PipelineRef = nullptr;
		std::vector<DescriptorHandle> DescriptorHandles;
		std::vector<uint8_t> PushConstantBuffer;
	};

}
