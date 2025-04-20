#include "Renderer.hpp"
#include "Application.hpp"


#include <backends/imgui_impl_vulkan.h>

namespace Cobalt
{

	struct Vertex
	{
		glm::vec3 aPosition;
		glm::vec3 aNormal;
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
			vertices[0] = { .aPosition = {-0.5f,  0.5f, 0.5f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[1] = { .aPosition = {-0.5f, -0.5f, 0.5f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[2] = { .aPosition = {  0.5f, -0.5f, 0.5f }, .aNormal = { 0.0f, 0.0f, 1.0f } };
			vertices[3] = { .aPosition = {  0.5f,  0.5f, 0.5f }, .aNormal = { 0.0f, 0.0f, 1.0f } };

			// back 
			vertices[4] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[5] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[6] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aNormal = { 0.0f, 0.0f, -1.0f } };
			vertices[7] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aNormal = { 0.0f, 0.0f, -1.0f } };

			// right 
			vertices[8] = { .aPosition  = {  0.5f,  0.5f,  0.5f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[9] = { .aPosition  = {  0.5f, -0.5f,  0.5f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[10] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aNormal = { 1.0f, 0.0f, 0.0f } };
			vertices[11] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aNormal = { 1.0f, 0.0f, 0.0f } };

			// left
			vertices[12] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[13] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[14] = { .aPosition = { -0.5f, -0.5f,  0.5f }, .aNormal = { -1.0f, 0.0f, 0.0f } };
			vertices[15] = { .aPosition = { -0.5f,  0.5f,  0.5f }, .aNormal = { -1.0f, 0.0f, 0.0f } };

			// top
			vertices[16] = { .aPosition = { -0.5f,  0.5f, -0.5f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[17] = { .aPosition = { -0.5f,  0.5f,  0.5f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[18] = { .aPosition = {  0.5f,  0.5f,  0.5f }, .aNormal = { 0.0f, 1.0f, 0.0f } };
			vertices[19] = { .aPosition = {  0.5f,  0.5f, -0.5f }, .aNormal = { 0.0f, 1.0f, 0.0f } };

			// bottom
			vertices[20] = { .aPosition = { -0.5f, -0.5f,  0.5f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[21] = { .aPosition = { -0.5f, -0.5f, -0.5f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[22] = { .aPosition = {  0.5f, -0.5f, -0.5f }, .aNormal = { 0.0f, -1.0f, 0.0f } };
			vertices[23] = { .aPosition = {  0.5f, -0.5f,  0.5f }, .aNormal = { 0.0f, -1.0f, 0.0f } };

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

		// Create pipeline

		{
			PipelineInfo pipelineInfo = {
				.Shader = std::make_shared<Shader>("CobaltApp/Assets/Shaders/CubeShader.glsl"),
				.PrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.EnableDepthTesting = true
			};

			sData->CubePipeline = std::make_shared<Pipeline>(pipelineInfo, sData->MainRenderPass);
		}

		// Create scene & material uniform buffers & descriptors

		{
			// Uniform buffers

			sData->MappedSceneData = new SceneData;

			sData->SceneDataUniformBuffer = std::make_unique<VulkanBuffer>(sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			sData->SceneDataUniformBuffer->Map(0, sizeof(SceneData), (void**)&sData->MappedSceneData);

			sData->MappedMaterialData = new MaterialData;
			sData->MaterialDataStorageBuffer = std::make_unique<VulkanBuffer>(sData->MaxMaterialCount * sizeof(MaterialData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			sData->MaterialDataStorageBuffer->Map(0, sData->MaxMaterialCount * sizeof(MaterialData), (void**)&sData->MappedMaterialData);

			sData->MappedObjectData = new ObjectData;
			sData->ObjectDataStorageBuffer = std::make_unique<VulkanBuffer>(sData->MaxObjectCount * sizeof(ObjectData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			sData->ObjectDataStorageBuffer->Map(0, sData->MaxObjectCount * sizeof(ObjectData), (void**)&sData->MappedObjectData);

			// Set descriptor bindings

			sData->SceneDataDescriptorSet = sData->CubePipeline->AllocateDescriptorSet(0, GraphicsContext::Get().GetDescriptorPool());
			sData->SceneDataDescriptorSet->SetBufferBinding(0, sData->SceneDataUniformBuffer.get());

			sData->MaterialDataDescriptorSet = sData->CubePipeline->AllocateDescriptorSet(1, GraphicsContext::Get().GetDescriptorPool());
			sData->MaterialDataDescriptorSet->SetBufferBinding(0, sData->MaterialDataStorageBuffer.get());

			sData->ObjectDataDescriptorSet = sData->CubePipeline->AllocateDescriptorSet(2, GraphicsContext::Get().GetDescriptorPool());
			sData->ObjectDataDescriptorSet->SetBufferBinding(0, sData->ObjectDataStorageBuffer.get());
		}
	}

	void Renderer::Shutdown()
	{
		sData->SceneDataUniformBuffer->Unmap();
		sData->SceneDataUniformBuffer.reset();

		sData->ObjectDataStorageBuffer->Unmap();
		sData->ObjectDataStorageBuffer.reset();

		sData->MaterialDataStorageBuffer->Unmap();
		sData->MaterialDataStorageBuffer.reset();

		sData->MappedSceneData = nullptr;
		sData->MappedObjectData = nullptr;
		sData->MappedMaterialData = nullptr;

		vkDestroyRenderPass(GraphicsContext::Get().GetDevice(), sData->MainRenderPass, nullptr);

		for (VkFramebuffer framebuffer : sData->Framebuffers)
			vkDestroyFramebuffer(GraphicsContext::Get().GetDevice(), framebuffer, nullptr);
		
		delete sData;
		sData = nullptr;
	}

	void Renderer::OnResize()
	{
		CreateOrRecreateDepthTexture();
		CreateOrRecreateFramebuffers();
	}

	uint32_t Renderer::RegisterMaterial(const MaterialData& materialData)
	{
		uint32_t materialIndex = sData->MaterialIndex++;

		sData->MappedMaterialData[materialIndex] = materialData;

		return materialIndex;
	}

	MaterialData& Renderer::GetMaterial(uint32_t materialIndex)
	{
		return sData->MappedMaterialData[materialIndex];
	}

	void Renderer::BeginScene(const SceneData& scene)
	{
		memset(sData->Objects.data(), 0, sizeof(sData->Objects));
		sData->ObjectIndex = 0;

		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		// Upload scene data

		memcpy(sData->MappedSceneData, &scene, sizeof(SceneData));
		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sData->CubePipeline->GetPipelineLayout(), 0, 1, &sData->SceneDataDescriptorSet, 0, nullptr);
		sData->SceneDataDescriptorSet->Bind(commandBuffer);
	}

	void Renderer::EndScene()
	{
		const Swapchain& swapchain = GraphicsContext::Get().GetSwapchain();

		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();
		
		// Viewport & scissor

		VkExtent2D extent = GraphicsContext::Get().GetSwapchain().GetExtent();

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

		// Render objects

		sData->MaterialDataDescriptorSet->Bind(commandBuffer);

		memcpy(sData->MappedObjectData, sData->Objects.data(), sData->ObjectIndex * sizeof(ObjectData));
		sData->ObjectDataDescriptorSet->Bind(commandBuffer);

		VkBuffer vertexBuffer = sData->VertexBuffer->GetBuffer();
		VkBuffer indexBuffer  = sData->IndexBuffer->GetBuffer();
		VkDeviceSize offset = 0;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sData->CubePipeline->GetPipeline());
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (uint32_t i = 0; i < sData->ObjectIndex; i++)
		{
			vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, i);
		}

		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::DrawCube(const Transform& transform, uint32_t materialIndex)
	{
		ObjectData objectData;
		objectData.Transform = transform.GetTransform();
		objectData.MaterialIndex = materialIndex;

		sData->Objects[sData->ObjectIndex++] = objectData;
	}

	void Renderer::CreateOrRecreateDepthTexture()
	{
		uint32_t width  = GraphicsContext::Get().GetSwapchain().GetExtent().width;
		uint32_t height = GraphicsContext::Get().GetSwapchain().GetExtent().height;

		if (sData->DepthTexture)
			sData->DepthTexture->Recreate(width, height);
		else
			sData->DepthTexture = std::make_unique<Texture>(width, height, VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
			VkImageView attachments[2] = { swapchain.GetBackBufferViews()[i], sData->DepthTexture->GetImageView() };
			createInfo.attachmentCount = 2;
			createInfo.pAttachments = attachments;

			VK_CALL(vkCreateFramebuffer(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &sData->Framebuffers[i]));
		}
	}

}