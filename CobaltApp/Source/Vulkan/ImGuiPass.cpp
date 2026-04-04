#include "copch.hpp"
#include "ImGuiPass.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Cobalt
{

	ImGuiPass::ImGuiPass()
	{
		CO_PROFILE_FN();
	}

	ImGuiPass::~ImGuiPass()
	{
		CO_PROFILE_FN();
	}

	void ImGuiPass::Init()
	{
		CO_PROFILE_FN();

		// Init ImGui

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

		GraphicsContext& graphicsContext = GraphicsContext::Get();

#if 0
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

				sData->CommandBuffers[i] = GraphicsContext::Get().AllocateCommandBuffer(sData->CommandPools[i]);
			}
		}
#endif

		// Create render pass

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

		VK_CALL(vkCreateRenderPass(graphicsContext.GetDevice(), &info, nullptr, &mRenderPass));

		// Create framebuffers

		CreateOrRecreateFramebuffers();

		// Create descriptor pool

		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = 2,
			.poolSizeCount = 1,
			.pPoolSizes = poolSizes
		};

		VK_CALL(vkCreateDescriptorPool(graphicsContext.GetDevice(), &descriptorPoolCreateInfo, nullptr, &mDescriptorPool));


		// Init imgui vulkan backend

		ImGui_ImplVulkan_InitInfo initInfo = {
			.Instance = graphicsContext.GetInstance(),
			.PhysicalDevice = graphicsContext.GetPhysicalDevice(),
			.Device = graphicsContext.GetDevice(),
			.QueueFamily = (uint32_t)graphicsContext.GetQueueFamily(),
			.Queue = graphicsContext.GetQueue(),
			.PipelineCache = VK_NULL_HANDLE,
			.DescriptorPool = mDescriptorPool,
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
		ImGui_ImplVulkan_Init(&initInfo, mRenderPass);

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

	void ImGuiPass::Shutdown()
	{
		CO_PROFILE_FN();

		vkDestroyRenderPass(GraphicsContext::Get().GetDevice(), mRenderPass, nullptr);

		//for (uint32_t i = 0; i < sData->CommandPools.size(); i++)
			//vkDestroyCommandPool(GraphicsContext::Get().GetDevice(), sData->CommandPools[i], nullptr);

		for (VkFramebuffer framebuffer : mFramebuffers)
			vkDestroyFramebuffer(GraphicsContext::Get().GetDevice(), framebuffer, nullptr);

		//sData->CommandPools.clear();
		//sData->CommandBuffers.clear();
		mFramebuffers.clear();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiPass::BeginFrame()
	{
		CO_PROFILE_FN();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiPass::EndFrame()
	{
		CO_PROFILE_FN();

		ImGui::Render();

		// Render viewports

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}
	}

	void ImGuiPass::RenderFrame(VkCommandBuffer commandBuffer)
	{
		CO_PROFILE_FN();

		uint32_t backBufferIndex = GraphicsContext::Get().GetSwapchain().GetBackBufferIndex();
		uint32_t frameIndex = GraphicsContext::Get().GetFrameIndex();

		VkFramebuffer framebuffer = mFramebuffers[backBufferIndex];
		//VkCommandPool commandPool = sData->CommandPools[frameIndex];

#if 0
		// Restart command buffer

		sData->ActiveCommandBuffer = sData->CommandBuffers[frameIndex];

		CO_PROFILE_COMMAND_BUFFER(sData->ActiveCommandBuffer);
		CO_PROFILE_GPU_EVENT("ImGui Rendering");

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		VK_CALL(vkBeginCommandBuffer(sData->ActiveCommandBuffer, &beginInfo));
#endif

		// Submit render pass

		VkRenderPassBeginInfo renderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = mRenderPass,
			.framebuffer = framebuffer,
			.renderArea = { .extent = GraphicsContext::Get().GetSwapchain().GetExtent() },
			.clearValueCount = 0,
			.pClearValues = nullptr,
		};

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		vkCmdEndRenderPass(commandBuffer);

		// End command buffer

		//VK_CALL(vkEndCommandBuffer(sData->ActiveCommandBuffer));

		// Render viewports

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiPass::OnResize()
	{
		CO_PROFILE_FN();

		CreateOrRecreateFramebuffers();
	}

	void ImGuiPass::CreateOrRecreateFramebuffers()
	{
		CO_PROFILE_FN();

		if (mFramebuffers.size() > 0)
		{
			for (VkFramebuffer framebuffer : mFramebuffers)
				vkDestroyFramebuffer(GraphicsContext::Get().GetDevice(), framebuffer, nullptr);

			mFramebuffers.clear();
		}

		const Swapchain& swapchain = GraphicsContext::Get().GetSwapchain();

		mFramebuffers.resize(swapchain.GetBackBufferCount());

		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.flags = 0,
			.renderPass = mRenderPass,
			.width = swapchain.GetExtent().width,
			.height = swapchain.GetExtent().height,
			.layers = 1,
		};

		for (uint32_t i = 0; i < swapchain.GetBackBufferCount(); i++)
		{
			VkImageView attachments[1] = { swapchain.GetBackBufferViews()[i] };
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachments;

			VK_CALL(vkCreateFramebuffer(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &mFramebuffers[i]));
		}
	}

}
