#include "copch.hpp"
#include "LightingPass.hpp"
#include "GraphicsContext.hpp"
#include "Renderer.hpp"
#include "RenderGraph.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	LightingPass::LightingPass()
		: RenderPass("Lighting Pass", "Deferred\\LightingPass.slang", (RenderPassFlags)RenderPassFlagBits::SideAffect)
	{
		CO_PROFILE_FN();
	}

	LightingPass::~LightingPass()
	{
		CO_PROFILE_FN();
	}

	void LightingPass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mPositionAttachment  = builder.GetResource("Position Attachment");
		mBaseColorAttachment = builder.GetResource("Base Color Attachment");
		mNormalAttachment    = builder.GetResource("Normal Attachment");
		mOCRAttachment       = builder.GetResource("OCR Attachment");
		mEmissiveAttachment  = builder.GetResource("Emissive Attachment");

		auto backbufferAttachment = builder.GetResource("BackBuffer Attachment");

		builder.AddDependency(mPositionAttachment, RGAccessType::ShaderRead);
		builder.AddDependency(mBaseColorAttachment, RGAccessType::ShaderRead);
		builder.AddDependency(mNormalAttachment, RGAccessType::ShaderRead);
		builder.AddDependency(mOCRAttachment, RGAccessType::ShaderRead);
		builder.AddDependency(mEmissiveAttachment, RGAccessType::ShaderRead);
		builder.AddDependency(backbufferAttachment, RGAccessType::Present);

		// Build pipeline

		PipelineInfo lightingPassPipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("Deferred\\LightingPass.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ true, GraphicsContext::Get().GetSwapchain().GetSurfaceFormat().format }
			}
		};

		mLightingPipelineBindings = PipelineBindings(GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Lighting", lightingPassPipelineInfo));
	}

	void LightingPass::SetSkybox(const Cubemap* skybox)
	{
		CO_PROFILE_FN();

		mSkybox = skybox;

		// Build skybox pipeline

		PipelineInfo skyboxPipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("Deferred\\Skybox.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ true, GraphicsContext::Get().GetSwapchain().GetSurfaceFormat().format }
			}
		};

		mSkyboxPipelineBindings = PipelineBindings(GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Skybox", skyboxPipelineInfo));

		// Allocate buffers

		uint32_t frameCount = GraphicsContext::Get().GetFrameCount();
		mSkyboxUniformBuffers.resize(frameCount);

		for (uint32_t i = 0; i < frameCount; i++)
		{
			mSkyboxUniformBuffers[i] = VulkanBuffer::CreateMappedBuffer(sizeof(SkyboxUniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		}
	}

	void LightingPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, GraphicsContext::Get().GetSwapchain().GetExtent(), false);

		if (mSkybox)
			ExecuteSkyboxPass(commandBuffer, renderContext);

		ExecuteLightingPass(commandBuffer, renderContext);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);
	}

	void LightingPass::ExecuteSkyboxPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		uint32_t frameIndex = GraphicsContext::Get().GetFrameIndex();
		auto& descriptorBufferManager = GraphicsContext::Get().GetDescriptorBufferManager();

		SkyboxUniformBuffer skyboxUniformBufferData{};
		skyboxUniformBufferData.ProjectionMatrix = renderContext.ProjectionMatrix;
		skyboxUniformBufferData.ViewMatrix = renderContext.ViewMatrix;
		skyboxUniformBufferData.MeshVertices = mSkybox->GetMesh()->GetVertexBufferReference();

		mSkyboxUniformBuffers[frameIndex]->CopyData(&skyboxUniformBufferData, sizeof(SkyboxUniformBuffer));

		ShaderCursor shaderCursor = mSkyboxPipelineBindings.GetShaderCursor(frameIndex);
		shaderCursor.WriteField("uniforms", *mSkyboxUniformBuffers[frameIndex]);
		shaderCursor.WriteField("skybox", *mSkybox);
		shaderCursor.Finalize();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipelineBindings.PipelineRef->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mSkybox->GetMesh()->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		descriptorBufferManager.SetDescriptorBufferOffsets(commandBuffer, mSkyboxPipelineBindings.PipelineRef->GetPipelineLayout(), mSkyboxPipelineBindings.DescriptorHandles[frameIndex]);
		vkCmdDrawIndexed(commandBuffer, mSkybox->GetMesh()->GetIndices().size(), 1, 0, 0, 0);
	}

	void LightingPass::ExecuteLightingPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		uint32_t frameIndex = GraphicsContext::Get().GetFrameIndex();
		auto& descriptorBufferManager = GraphicsContext::Get().GetDescriptorBufferManager();
		auto& renderGraph = Renderer::GetRenderGraph();

		ShaderCursor shaderCursor = mLightingPipelineBindings.GetShaderCursor(frameIndex);
		shaderCursor.Field("scene").Write(*renderContext.SceneBuffer);
		shaderCursor.Field("gBuffers")
			.WriteField("SamplerPosition", *renderGraph.GetResource(mPositionAttachment))
			.WriteField("SamplerBaseColor", *renderGraph.GetResource(mBaseColorAttachment))
			.WriteField("SamplerNormal", *renderGraph.GetResource(mNormalAttachment))
			.WriteField("SamplerOcclusionRoughnessMetallic", *renderGraph.GetResource(mOCRAttachment))
			.WriteField("SamplerEmissive", *renderGraph.GetResource(mEmissiveAttachment));
		shaderCursor.WriteField("brdfLUT", *renderContext.BRDFLUT);
		shaderCursor.WriteField("irradianceMap", *renderContext.IrradianceCube);
		shaderCursor.WriteField("prefilteredMap", *renderContext.PrefilteredEnvironmentMap);
		shaderCursor.Finalize();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mLightingPipelineBindings.PipelineRef->GetPipeline());
		descriptorBufferManager.SetDescriptorBufferOffsets(commandBuffer, mLightingPipelineBindings.PipelineRef->GetPipelineLayout(), mLightingPipelineBindings.DescriptorHandles[frameIndex]);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

}