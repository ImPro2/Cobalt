#include "ImGuiBackend.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Cobalt
{

	void ImGuiBackend::Init()
	{
		sData = new ImGuiBackendData();

		// Init ImGui


		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_ViewportsEnable;

			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}
		}

		ImGuiIO& io = ImGui::GetIO();
		GraphicsContext& graphicsContext = GraphicsContext::Get();

		// Create command pools & buffers

		{
			sData->CommandPools.resize(graphicsContext.GetFrameCount());
			sData->CommandBuffers.resize(graphicsContext.GetFrameCount());

			for (uint32_t i = 0; i < graphicsContext.GetFrameCount(); i++)
			{
				VkCommandPoolCreateInfo commandPoolCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
					.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
					.queueFamilyIndex = (uint32_t)graphicsContext.GetQueueFamily()
				};

				VK_CALL(vkCreateCommandPool(graphicsContext.GetDevice(), &commandPoolCreateInfo, nullptr, &sData->CommandPools[i]));

				VkCommandBufferAllocateInfo commandBufferAllocInfo = {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.commandPool = sData->CommandPools[i],
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};

				VK_CALL(vkAllocateCommandBuffers(graphicsContext.GetDevice(), &commandBufferAllocInfo, &sData->CommandBuffers[i]));
			}
		}

		// Create render pass

		{
			const Swapchain& swapchain = graphicsContext.GetSwapchain();

			VkAttachmentDescription attachment = {};
			attachment.format = swapchain.GetSurfaceFormat().format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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
			dependency.srcAccessMask = 0;//VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;

			VK_CALL(vkCreateRenderPass(graphicsContext.GetDevice(), &info, nullptr, &sData->ImGuiRenderPass));
		}

		// Create framebuffers

		CreateOrRecreateFramebuffers();

		// Init imgui vulkan backend

		{
			ImGui_ImplVulkan_InitInfo initInfo = {
				.Instance = graphicsContext.GetInstance(),
				.PhysicalDevice = graphicsContext.GetPhysicalDevice(),
				.Device = graphicsContext.GetDevice(),
				.QueueFamily = (uint32_t)graphicsContext.GetQueueFamily(),
				.Queue = graphicsContext.GetQueue(),
				.PipelineCache = VK_NULL_HANDLE,
				.DescriptorPool = graphicsContext.GetDescriptorPool(),
				.Subpass = 0,
				.MinImageCount = graphicsContext.GetSwapchain().GetBackBufferCount(),
				.ImageCount = graphicsContext.GetSwapchain().GetBackBufferCount(),
				.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
				.Allocator = nullptr,
				.CheckVkResultFn = [](VkResult result)
				{
					if (result == VK_SUCCESS)
						return;

					std::cerr << "ImGui Vulkan Error (result)\n";
				}
			};

			ImGui_ImplGlfw_InitForVulkan(Application::Get()->GetWindow().GetWindow(), true);
			ImGui_ImplVulkan_Init(&initInfo, sData->ImGuiRenderPass);

			ImFontConfig fontConfig;
			fontConfig.FontDataOwnedByAtlas = false;
			ImFont* robotoFont = io.Fonts->AddFontFromFileTTF("CobaltApp/Assets/Fonts/Roboto.ttf", 20.0f);
			io.FontDefault = robotoFont;

			// Upload fonts

			graphicsContext.SubmitSingleTimeCommands(graphicsContext.GetQueue(), [](VkCommandBuffer commandBuffer)
				{
					ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
				});

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	void ImGuiBackend::Shutdown()
	{
		vkDestroyRenderPass(GraphicsContext::Get().GetDevice(), sData->ImGuiRenderPass, nullptr);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		delete sData;
	}

	void ImGuiBackend::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiBackend::EndFrame()
	{
		ImGui::Render();

		// Render viewports

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}
	}

	void ImGuiBackend::RenderFrame()
	{
		uint32_t frameIndex = GraphicsContext::Get().GetFrameIndex();
		uint32_t backBufferIndex = GraphicsContext::Get().GetSwapchain().GetBackBufferIndex();

		VkCommandPool commandPool = sData->CommandPools[frameIndex];
		VkCommandBuffer commandBuffer = sData->CommandBuffers[frameIndex];
		VkFramebuffer framebuffer = sData->Framebuffers[backBufferIndex];

		// Start command buffer

		VK_CALL(vkResetCommandPool(GraphicsContext::Get().GetDevice(), commandPool, 0));

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		// Submit render pass

		VkRenderPassBeginInfo renderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = sData->ImGuiRenderPass,
			.framebuffer = framebuffer,
			.renderArea = { .extent = GraphicsContext::Get().GetSwapchain().GetExtent() },
			.clearValueCount = 0,
			.pClearValues = nullptr,
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		vkCmdEndRenderPass(commandBuffer);

		// End command buffer

		VK_CALL(vkEndCommandBuffer(commandBuffer));

		// Render viewports

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	VkCommandBuffer ImGuiBackend::GetActiveCommandBuffer()
	{
		return sData->CommandBuffers[GraphicsContext::Get().GetFrameIndex()];
	}

	void ImGuiBackend::OnResize()
	{
		CreateOrRecreateFramebuffers();
	}

	void ImGuiBackend::CreateOrRecreateFramebuffers()
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
			.renderPass = sData->ImGuiRenderPass,
			.width = swapchain.GetExtent().width,
			.height = swapchain.GetExtent().height,
			.layers = 1,
		};

		for (uint32_t i = 0; i < swapchain.GetBackBufferCount(); i++)
		{
			VkImageView attachments[1] = { swapchain.GetBackBufferViews()[i] };
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachments;

			VK_CALL(vkCreateFramebuffer(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &sData->Framebuffers[i]));
		}
	}

}
