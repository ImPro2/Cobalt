#include "Pipeline.hpp"
#include "GraphicsContext.hpp"
#include "Application.hpp"

namespace Cobalt
{

	Pipeline::Pipeline(const PipelineInfo& info, VkRenderPass renderPass)
		: mInfo(info), mRenderPass(renderPass)
	{
		Invalidate();
	}

	Pipeline::~Pipeline()
	{
		if (mPipelineLayout)
			vkDestroyPipelineLayout(GraphicsContext::Get().GetDevice(), mPipelineLayout, nullptr);

		if (mPipeline)
			vkDestroyPipeline(GraphicsContext::Get().GetDevice(), mPipeline, nullptr);
	}
	
	void Pipeline::Invalidate()
	{
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2];

		shaderStageCreateInfos[0] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = mInfo.VertexShader->GetShaderModule(),
			.pName = "main"
		};

		shaderStageCreateInfos[1] = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = mInfo.FragmentShader->GetShaderModule(),
			.pName = "main"
		};

		VkVertexInputBindingDescription vertexInputBindingDescription = {
			.binding = 0,
			.stride = mInfo.InputLayout.GetStride(),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		};

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(mInfo.InputLayout.GetAttributes().size());

		for (uint32_t i = 0; i < mInfo.InputLayout.GetAttributes().size(); i++)
		{
			const auto& attribute = mInfo.InputLayout.GetAttributes()[i];

			vertexInputAttributeDescriptions[i] = {
				.location = attribute.Location,
				.binding = 0,
				.format = attribute.Type,
				.offset = attribute.Offset
			};
		}

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.flags = 0,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &vertexInputBindingDescription,
			.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size(),
			.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.flags = 0,
			.topology = mInfo.PrimitiveTopology,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkViewport viewport = {
			.width  = (float)Application::Get()->GetWindow().GetWidth(),
			.height = (float)Application::Get()->GetWindow().GetHeight(),
		};

		VkRect2D scissor = {
			.offset = {},
			.extent = { (uint32_t)viewport.width, (uint32_t)viewport.height }
		};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.flags = 0,
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor
		};

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f
		};

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.flags = 0,
			.depthTestEnable = mInfo.EnableDepthTesting ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = mInfo.EnableDepthTesting ? VK_TRUE : VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {},
			.back = {},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.flags = 0,
			.logicOpEnable = VK_FALSE,
			.logicOp = {},
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachmentState,
		};

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]),
			.pDynamicStates = dynamicStates
		};

		VkPushConstantRange pushConstantRanges[1];
		pushConstantRanges[0] = {
			.stageFlags = mInfo.PushConstantShaderStage,
			.offset = 0,
			.size = mInfo.PushConstantSize,
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.flags = 0,
			.setLayoutCount = (uint32_t)mInfo.DescriptorSetLayouts.size(),
			.pSetLayouts = mInfo.DescriptorSetLayouts.data(),
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = pushConstantRanges
		};

		VK_CALL(vkCreatePipelineLayout(GraphicsContext::Get().GetDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.flags = 0,
			.stageCount = sizeof(shaderStageCreateInfos) / sizeof(shaderStageCreateInfos[0]),
			.pStages = shaderStageCreateInfos,
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilStateCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = mPipelineLayout,
			.renderPass = mRenderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0
		};

		VK_CALL(vkCreateGraphicsPipelines(GraphicsContext::Get().GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline));
	}

}