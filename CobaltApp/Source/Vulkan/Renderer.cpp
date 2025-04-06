#include "Renderer.hpp"
#include "Application.hpp"


#include <backends/imgui_impl_vulkan.h>

namespace Cobalt
{

	struct Vertex
	{
		alignas(16) glm::vec3 aPosition;
		alignas(16) glm::vec3 aColor;
		alignas(16) glm::vec3 aNormal;
	};

	void Renderer::Init()
	{
		if (sData)
			return;

		sData = new RendererData();

		// Init buffers

		{
			constexpr uint32_t squareCount = 6;
			constexpr uint32_t vertexCount = squareCount * 4;
			constexpr uint32_t indexCount = squareCount * 6;

			std::array<Vertex, vertexCount> vertices;

			// front
			vertices[0] = { .aPosition = {-0.5f,  0.5f, 0.5f }, .aColor = { 1.0f, 0.0f, 0.0f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[1] = { .aPosition = {-0.5f, -0.5f, 0.5f }, .aColor = { 1.0f, 0.0f, 0.0f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[2] = { .aPosition = {  0.5f, -0.5f, 0.5f }, .aColor = { 1.0f, 0.0f, 0.0f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[3] = { .aPosition = {  0.5f,  0.5f, 0.5f }, .aColor = { 1.0f, 0.0f, 0.0f }, .aNormal = { 0.0f, 0.0f, 1.0f } };

			// back 
			vertices[4] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 1.0f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[5] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 1.0f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[6] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 1.0f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[7] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 1.0f }, .aNormal = { 0.0f, 0.0f, -1.0f } };

			// right 
			vertices[8] = { .aPosition  = {  0.5f,  0.5f,  0.5f }, .aColor = { 0.0f, 1.0f, 0.0f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[9] = { .aPosition  = {  0.5f, -0.5f,  0.5f }, .aColor = { 0.0f, 1.0f, 0.0f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[10] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aColor = { 0.0f, 1.0f, 0.0f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[11] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aColor = { 0.0f, 1.0f, 0.0f }, .aNormal = { 1.0f, 0.0f, 0.0f } };

			// left
			vertices[12] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aColor = { 0.0f, 0.0f, 1.0f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[13] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aColor = { 0.0f, 0.0f, 1.0f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[14] = { .aPosition = { -0.5f, -0.5f,  0.5f }, .aColor = { 0.0f, 0.0f, 1.0f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[15] = { .aPosition = { -0.5f,  0.5f,  0.5f }, .aColor = { 0.0f, 0.0f, 1.0f }, .aNormal = { -1.0f, 0.0f, 0.0f } };

			// top
			vertices[16] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aColor = { 1.0f, 0.0f, 1.0f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[17] = { .aPosition = { -0.5f,  0.5f,  0.5f }, .aColor = { 1.0f, 0.0f, 1.0f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[18] = { .aPosition = {  0.5f,  0.5f,  0.5f }, .aColor = { 1.0f, 0.0f, 1.0f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[19] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aColor = { 1.0f, 0.0f, 1.0f }, .aNormal = { 0.0f, 1.0f, 0.0f } };

			// bottom
			vertices[20] = { .aPosition = { -0.5f, -0.5f,  0.5f }, .aColor = { 1.0f, 1.0f, 0.0f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[21] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 0.0f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[22] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aColor = { 1.0f, 1.0f, 0.0f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[23] = { .aPosition = {  0.5f, -0.5f,  0.5f }, .aColor = { 1.0f, 1.0f, 0.0f }, .aNormal = { 0.0f, -1.0f, 0.0f } };

			std::array<uint32_t, indexCount> indices;

			uint32_t offset = 0;
			for (uint32_t i = 0; i < indices.size(); i += 6)
			{
				indices[i + 0] = offset + 0;
				indices[i + 1] = offset + 1;
				indices[i + 2] = offset + 2;

				indices[i + 3] = offset + 2;
				indices[i + 4] = offset + 3;
				indices[i + 5] = offset + 0;

				offset += 4;
			}

			VulkanBuffer vertexStagingBuffer = VulkanBuffer(sizeof(vertices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			vertexStagingBuffer.CopyData(0, sizeof(vertices), vertices.data());

			VulkanBuffer indexStagingBuffer = VulkanBuffer(sizeof(indices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			indexStagingBuffer.CopyData(0, sizeof(indices), indices.data());

			sData->VertexBuffer = std::make_unique<VulkanBuffer>(sizeof(vertices), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			sData->IndexBuffer = std::make_unique<VulkanBuffer>(sizeof(indices), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			// Transfer

			GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
			{
				VkBufferCopy vertexBufferCopy = {
					.srcOffset = 0,
					.dstOffset = 0,
					.size = sData->VertexBuffer->GetMemoryRequirements().size,
				};

				VkBufferCopy indexBufferCopy = {
					.srcOffset = 0,
					.dstOffset = 0,
					.size = sData->IndexBuffer->GetMemoryRequirements().size,
				};

				vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.GetBuffer(), sData->VertexBuffer->GetBuffer(), 1, &vertexBufferCopy);
				vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.GetBuffer(), sData->IndexBuffer->GetBuffer(), 1, &indexBufferCopy);
			});
		}

		// Create depth texture

		CreateOrRecreateDepthTexture();

		// Create the Render Pass

		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = GraphicsContext::Get().GetSwapchain().GetSurfaceFormat().format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = VK_FORMAT_D16_UNORM_S8_UINT;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment};
			VkAttachmentReference attachmentRefs[2] = { colorAttachmentRef, depthAttachmentRef };

			VkSubpassDescription subpass1 = {};
			subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass1.colorAttachmentCount = 1;
			subpass1.pColorAttachments = &colorAttachmentRef;
			subpass1.pDepthStencilAttachment = &depthAttachmentRef;

			VkSubpassDependency dependency1 = {};
			dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency1.dstSubpass = 0;
			dependency1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency1.srcAccessMask = 0;
			dependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			
			VkSubpassDescription subpasses[1] = { subpass1 };
			VkSubpassDependency dependencies[1] = { dependency1 };

			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 2;
			info.pAttachments = attachments;
			info.subpassCount = 1;
			info.pSubpasses = subpasses;
			info.dependencyCount = 1;
			info.pDependencies = dependencies;

			VK_CALL(vkCreateRenderPass(GraphicsContext::Get().GetDevice(), &info, nullptr, &sData->MainRenderPass));
		}

		// Create framebuffers

		CreateOrRecreateFramebuffers();

		// Create scene data

		{
			// Uniform buffer

			sData->CurrentSceneData = new SceneData;

			sData->SceneDataUniformBuffer = std::make_unique<VulkanBuffer>(sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			sData->SceneDataUniformBuffer->Map(0, sizeof(SceneData), (void**)&sData->CurrentSceneData);

			// Descriptor set layout

			VkDescriptorSetLayoutBinding descSetLayoutBinding = {
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			};

			VkDescriptorSetLayoutBinding bindings[] = { descSetLayoutBinding };

			VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
				.pBindings = bindings
			};

			VK_CALL(vkCreateDescriptorSetLayout(GraphicsContext::Get().GetDevice(), &descSetLayoutCreateInfo, nullptr, &sData->SceneDataDescriptorSetLayout));

			// Descriptor set

			VkDescriptorSetAllocateInfo descSetAllocInfo = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = GraphicsContext::Get().GetDescriptorPool(),
				.descriptorSetCount = 1,
				.pSetLayouts = &sData->SceneDataDescriptorSetLayout
			};

			VK_CALL(vkAllocateDescriptorSets(GraphicsContext::Get().GetDevice(), &descSetAllocInfo, &sData->SceneDataDescriptorSet));

			// Update descriptor sets

			VkDescriptorBufferInfo descBufferInfo = {
				.buffer = sData->SceneDataUniformBuffer->GetBuffer(),
				.offset = 0,
				.range = sizeof(SceneData)
			};

			VkWriteDescriptorSet writeDescSet = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = sData->SceneDataDescriptorSet,
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &descBufferInfo,
			};

			VkWriteDescriptorSet writeDescSets[] = { writeDescSet };

			vkUpdateDescriptorSets(GraphicsContext::Get().GetDevice(), sizeof(writeDescSets) / sizeof(writeDescSets[0]), writeDescSets, 0, nullptr);
		}

		// Create pipeline

		{
			PipelineInfo pipelineInfo = {
				.InputLayout = VertexInputLayout({
					VertexInputLayoutAttribute(0, VK_FORMAT_R32G32B32A32_SFLOAT),
					VertexInputLayoutAttribute(1, VK_FORMAT_R32G32B32A32_SFLOAT),
					VertexInputLayoutAttribute(2, VK_FORMAT_R32G32B32A32_SFLOAT)
				}),
				.VertexShader = std::make_shared<Shader>("CobaltApp/Assets/Shaders/VertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT),
				.FragmentShader = std::make_shared<Shader>("CobaltApp/Assets/Shaders/FragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
				.PrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.EnableDepthTesting = true,
				.PushConstantSize = sizeof(PushConstants),
				.DescriptorSetLayouts = { sData->SceneDataDescriptorSetLayout }
			};

			sData->CubePipeline = std::make_shared<Pipeline>(pipelineInfo, sData->MainRenderPass);
		}
	}

	void Renderer::Shutdown()
	{
		vkDestroyDescriptorSetLayout(GraphicsContext::Get().GetDevice(), sData->SceneDataDescriptorSetLayout, nullptr);
		vkFreeDescriptorSets(GraphicsContext::Get().GetDevice(), GraphicsContext::Get().GetDescriptorPool(), 1, &sData->SceneDataDescriptorSet);

		sData->SceneDataUniformBuffer->Unmap();
		sData->SceneDataUniformBuffer.reset();

		//delete sData->CurrentSceneData;
		sData->CurrentSceneData = nullptr;

		vkDestroyImage(GraphicsContext::Get().GetDevice(), sData->DepthTexture, nullptr);
		vkDestroyImageView(GraphicsContext::Get().GetDevice(), sData->DepthTextureView, nullptr);
		vkFreeMemory(GraphicsContext::Get().GetDevice(), sData->DepthTextureMemory, nullptr);

		vkDestroyRenderPass(GraphicsContext::Get().GetDevice(), sData->MainRenderPass, nullptr);

		for (VkFramebuffer framebuffer : sData->Framebuffers)
			vkDestroyFramebuffer(GraphicsContext::Get().GetDevice(), framebuffer, nullptr);
		
		delete sData;
	}

	void Renderer::OnResize()
	{
		CreateOrRecreateDepthTexture();
		CreateOrRecreateFramebuffers();
	}

	void Renderer::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraTranslation, const glm::vec3& lightPosition, const glm::vec3& lightColor)
	{
		const Swapchain& swapchain = GraphicsContext::Get().GetSwapchain();

		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		// Begin render pass

		VkClearValue clearValues[2] = {};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = {1.0f, 0};

		VkRenderPassBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = sData->MainRenderPass,
			.framebuffer = sData->Framebuffers[swapchain.GetBackBufferIndex()],
			.renderArea = { .extent = swapchain.GetExtent() },
			.clearValueCount = 2,
			.pClearValues = clearValues,
		};

		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Upload scene data

		VkExtent2D extent = GraphicsContext::Get().GetSwapchain().GetExtent();

		SceneData sceneData = {};
		sceneData.ViewProjection = viewProjectionMatrix;
		sceneData.LightPosition = lightPosition;
		sceneData.LightColor = lightColor;
		sceneData.CameraPosition = cameraTranslation;
 
		memcpy(sData->CurrentSceneData, &sceneData, sizeof(SceneData));

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sData->CubePipeline->GetPipelineLayout(), 0, 1, &sData->SceneDataDescriptorSet, 0, nullptr);

		// Viewport & scissor

		VkViewport viewport = {
			.x = 0,
			.y = (float)extent.height,
			.width = (float)extent.width,
			.height = -(float)extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		VkRect2D scissor = {
			.extent = extent
		};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::EndScene()
	{
		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::DrawCube(const Transform& transform)
	{
		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		PushConstants pushConstants;
		pushConstants.CubeTransform = transform.GetTransform();

		vkCmdPushConstants(commandBuffer, sData->CubePipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);

		VkBuffer vertexBuffer = sData->VertexBuffer->GetBuffer();
		VkBuffer indexBuffer  = sData->IndexBuffer->GetBuffer();

		VkDeviceSize offset = 0;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sData->CubePipeline->GetPipeline());
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
	}

	void Renderer::CreateOrRecreateDepthTexture()
	{
		uint32_t width  = GraphicsContext::Get().GetSwapchain().GetExtent().width;
		uint32_t height = GraphicsContext::Get().GetSwapchain().GetExtent().height;

		if (sData->DepthTexture)
			vkDestroyImage(GraphicsContext::Get().GetDevice(), sData->DepthTexture, nullptr);

		if (sData->DepthTextureView)
			vkDestroyImageView(GraphicsContext::Get().GetDevice(), sData->DepthTextureView, nullptr);
		
		if (sData->DepthTextureMemory)
			vkFreeMemory(GraphicsContext::Get().GetDevice(), sData->DepthTextureMemory, nullptr);

		VkImageCreateInfo imageCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D16_UNORM_S8_UINT,
			.extent = { width, height, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		VK_CALL(vkCreateImage(GraphicsContext::Get().GetDevice(), &imageCreateInfo, nullptr, &sData->DepthTexture));

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(GraphicsContext::Get().GetDevice(), sData->DepthTexture, &memoryRequirements);

		VkMemoryAllocateInfo memoryAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memoryRequirements.size,
			.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, GraphicsContext::Get().GetPhysicalDevice())
		};

		VK_CALL(vkAllocateMemory(GraphicsContext::Get().GetDevice(), &memoryAllocateInfo, nullptr, &sData->DepthTextureMemory));

		VK_CALL(vkBindImageMemory(GraphicsContext::Get().GetDevice(), sData->DepthTexture, sData->DepthTextureMemory, 0));

		VkImageViewCreateInfo imageViewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.flags = 0,
			.image = sData->DepthTexture,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = imageCreateInfo.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VK_CALL(vkCreateImageView(GraphicsContext::Get().GetDevice(), &imageViewCreateInfo, nullptr, &sData->DepthTextureView));
	}

	void Renderer::CreateOrRecreateFramebuffers()
	{
		if (sData->Framebuffers.size() > 0)
		{
			for (VkFramebuffer framebuffer : sData->Framebuffers)
				vkDestroyFramebuffer(GraphicsContext::Get().GetDevice(), framebuffer, nullptr);

			sData->Framebuffers.clear();
		}

		const Swapchain& swapchain = GraphicsContext::Get().GetSwapchain();

		sData->Framebuffers.resize(swapchain.GetBackBufferCount());

		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.flags = 0,
			.renderPass = sData->MainRenderPass,
			.width = swapchain.GetExtent().width,
			.height = swapchain.GetExtent().height,
			.layers = 1,
		};

		for (uint32_t i = 0; i < swapchain.GetBackBufferCount(); i++)
		{
			VkImageView attachments[2] = { swapchain.GetBackBufferViews()[i], sData->DepthTextureView };
			createInfo.attachmentCount = 2;
			createInfo.pAttachments = attachments;

			VK_CALL(vkCreateFramebuffer(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &sData->Framebuffers[i]));
		}
	}

}