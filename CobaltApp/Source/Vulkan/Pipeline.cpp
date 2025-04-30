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
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

		auto shaderModuleMap = mInfo.Shader->GetShaderModules();

		for (auto [stage, module] : shaderModuleMap)
		{
			shaderStageCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.flags = 0,
				.stage = (VkShaderStageFlagBits)stage,
				.module = module,
				.pName = "main"
			});
		}

		const auto& vertexInputBindingDescription = mInfo.Shader->GetVertexInputBindingDescription();
		const auto& vertexInputAttributeDescriptions = mInfo.Shader->GetVertexInputAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.flags = 0
		};

		if (!vertexInputAttributeDescriptions.empty())
		{
			vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
			vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
			vertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size();
			vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
		}

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

		const auto& descriptorSetLayouts = mInfo.Shader->GetDescriptorSetLayouts();
		const auto& pushConstantRanges   = mInfo.Shader->GetPushConstantRanges();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.flags = 0,
			.setLayoutCount = (uint32_t)descriptorSetLayouts.size(),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = (uint32_t)pushConstantRanges.size(),
			.pPushConstantRanges = pushConstantRanges.data()
		};

		VK_CALL(vkCreatePipelineLayout(GraphicsContext::Get().GetDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.flags = 0,
			.stageCount = (uint32_t)shaderStageCreateInfos.size(),
			.pStages = shaderStageCreateInfos.data(),
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

	std::vector<VulkanDescriptorSet*> Pipeline::AllocateDescriptorSets(VkDescriptorPool descriptorPool, uint32_t set, uint32_t count)
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts(count);

		for (uint32_t i = 0; i < count; i++)
		{
			descriptorSetLayouts[i] = mInfo.Shader->GetDescriptorSetLayouts()[set];
		}

		VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = count,
			.pSetLayouts = descriptorSetLayouts.data()
		};

		std::vector<VkDescriptorSet> descriptorSetHandles(count);

		VK_CALL(vkAllocateDescriptorSets(GraphicsContext::Get().GetDevice(), &descriptorSetAllocInfo, descriptorSetHandles.data()));

		std::vector<VulkanDescriptorSet*> descriptorSets(count);

		for (uint32_t i = 0; i < count; i++)
		{
			mDescriptorSets.push_back(std::make_unique<VulkanDescriptorSet>(set, descriptorSetHandles[i], mPipelineLayout));
			descriptorSets[i] = mDescriptorSets[mDescriptorSets.size() - 1].get();
		}

		return descriptorSets;
	}

}