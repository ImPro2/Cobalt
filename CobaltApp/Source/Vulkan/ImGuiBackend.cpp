#include "ImGuiBackend.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Cobalt
{

	void ImGuiBackend::Init()
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

		GraphicsContext& graphicsContext = GraphicsContext::Get();

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
		ImGui_ImplVulkan_Init(&initInfo, Renderer::GetMainRenderPass());

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

	void ImGuiBackend::Shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
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

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

}
