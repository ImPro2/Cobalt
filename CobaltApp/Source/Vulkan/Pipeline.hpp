#pragma once
#include "Shader.hpp"
#include "InputLayout.hpp"
#include <vulkan/vulkan.h>
#include <memory>

namespace Cobalt
{


	struct PipelineInfo
	{
		VertexInputLayout InputLayout;

		std::shared_ptr<Shader> VertexShader;
		std::shared_ptr<Shader> FragmentShader;

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

	public:
		VkPipeline GetPipeline() const { return mPipeline; }

	private:
		PipelineInfo mInfo;
		VkRenderPass mRenderPass;

		VkPipeline mPipeline;
		VkPipelineLayout mPipelineLayout;
	};

}
