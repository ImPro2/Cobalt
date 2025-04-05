#include "Renderer.hpp"
#include "Application.hpp"

namespace Cobalt
{

	struct Vertex
	{
		alignas(16) glm::vec3 aPosition;
		alignas(16) glm::vec3 aColor;
	};

	void Renderer::Init()
	{
		if (sData)
			return;

		sData = new RendererData();

		// Init buffers

		{
			std::array<Vertex, 4> vertices;
			vertices[0].aPosition = { -0.5f,  0.5f, 0.0f };
			vertices[0].aColor    = {  1.0f,  0.0f, 0.0f };
			vertices[1].aPosition = { -0.5f, -0.5f, 0.0f };
			vertices[1].aColor    = {  0.0f,  1.0f, 0.0f };
			vertices[2].aPosition = {  0.5f, -0.5f, 0.0f };
			vertices[2].aColor    = {  0.0f,  0.0f, 1.0f };
			vertices[3].aPosition = {  0.5f,  0.5f, 0.0f };
			vertices[3].aColor    = {  1.0f,  1.0f, 1.0f };

			std::array<uint32_t, 6> indices = { 0, 1, 2,  2, 3, 0};

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

		// Create the Render Pass

		{
			VkAttachmentDescription attachment = {};
			attachment.format = GraphicsContext::Get().GetSwapchain().GetSurfaceFormat().format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			VkAttachmentReference color_attachment = {};
			color_attachment.attachment = 0;
			color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment;
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			VK_CALL(vkCreateRenderPass(GraphicsContext::Get().GetDevice(), &info, nullptr, &sData->MainRenderPass));
		}

		// Create framebuffers

		CreateOrRecreateFramebuffers();

		// Create pipeline

		{
			PipelineInfo pipelineInfo = {
				.InputLayout = VertexInputLayout({
					VertexInputLayoutAttribute(0, VK_FORMAT_R32G32B32A32_SFLOAT),
					VertexInputLayoutAttribute(1, VK_FORMAT_R32G32B32A32_SFLOAT)
				}),
				.VertexShader = std::make_shared<Shader>("CobaltApp/Assets/Shaders/VertexShader.spv", VK_SHADER_STAGE_VERTEX_BIT),
				.FragmentShader = std::make_shared<Shader>("CobaltApp/Assets/Shaders/FragmentShader.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
				.PrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.EnableDepthTesting = false
			};

			sData->TrianglePipeline = std::make_shared<Pipeline>(pipelineInfo, sData->MainRenderPass);
		}
	}

	void Renderer::Shutdown()
	{
		delete sData;
	}

	void Renderer::OnResize()
	{
		CreateOrRecreateFramebuffers();
	}

	void Renderer::BeginScene()
	{
		const Swapchain& swapchain = GraphicsContext::Get().GetSwapchain();

		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		VkClearValue clearValues = {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}};

		VkRenderPassBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = sData->MainRenderPass,
			.framebuffer = sData->Framebuffers[swapchain.GetBackBufferIndex()],
			.renderArea = { .extent = swapchain.GetExtent() },
			.clearValueCount = 1,
			.pClearValues = &clearValues,
		};

		vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void Renderer::EndScene()
	{
		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::DrawTriangle()
	{
		VkCommandBuffer commandBuffer = GraphicsContext::Get().GetActiveCommandBuffer();

		float width  = (float)Application::Get()->GetWindow().GetWidth();
		float height = (float)Application::Get()->GetWindow().GetHeight();

		VkViewport viewport = {
			.x = 0,
			.y = height,
			.width = width,
			.height = -height,
		};

		VkRect2D scissor = {
			.extent = { (uint32_t)width, (uint32_t)height }
		};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffer = sData->VertexBuffer->GetBuffer();
		VkBuffer indexBuffer  = sData->IndexBuffer->GetBuffer();

		VkDeviceSize offset = 0;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sData->TrianglePipeline->GetPipeline());
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
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
			.attachmentCount = 1,
			.width = swapchain.GetExtent().width,
			.height = swapchain.GetExtent().height,
			.layers = 1,
		};

		for (uint32_t i = 0; i < swapchain.GetBackBufferCount(); i++)
		{
			VkImageView attachment[1] = { swapchain.GetBackBufferViews()[i] };
			createInfo.pAttachments = attachment;

			VK_CALL(vkCreateFramebuffer(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &sData->Framebuffers[i]));
		}
	}

}