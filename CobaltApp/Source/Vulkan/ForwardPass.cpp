#include "copch.hpp"
#include "ForwardPass.hpp"
#include "RenderGraph.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	ForwardPass::ForwardPass()
		: RenderPass("Forward Pass", "Forward\\ForwardPass.slang", (RenderPassFlags)RenderPassFlagBits::MeshPass)
	{
		CO_PROFILE_FN();
	}

	ForwardPass::~ForwardPass()
	{
		CO_PROFILE_FN();
	}

	void ForwardPass::SetSkybox(const Cubemap* skybox)
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

	void ForwardPass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		auto depthStencilAttachment = builder.DeclareResource("Depth Stencil Attachment", { RGResourceType::DepthAttachment });
		auto backbufferAttachment   = builder.GetResource("BackBuffer Attachment");

		builder.AddDependency(depthStencilAttachment, RGAccessType::DepthAttachment);
		builder.AddDependency(backbufferAttachment, RGAccessType::Present);

		builder.SetClearColor(backbufferAttachment);
	}

	void ForwardPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, GraphicsContext::Get().GetSwapchain().GetExtent(), false);

		if (mSkybox)
			ExecuteSkyboxPass(commandBuffer, renderContext);

		ExecuteForwardPass(commandBuffer, renderContext);

		mRenderGraph.EndPass(commandBuffer, mPassHandle);
	}

	void ForwardPass::ExecuteSkyboxPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
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

	void ForwardPass::ExecuteForwardPass(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		uint32_t frameIndex = GraphicsContext::Get().GetFrameIndex();
		const ShaderEffect* lastShaderEffect = nullptr;

		for (const auto& draw : renderContext.DrawCalls)
		{
			const ShaderEffect* shaderEffect = draw.Material->GetShaderEffect();

			if (shaderEffect == lastShaderEffect)
				continue;

			lastShaderEffect = shaderEffect;

			ShaderCursor shaderCursor = shaderEffect->GetShaderCursor(mName, frameIndex);
			shaderCursor.WriteField("gScene", *renderContext.SceneBuffer);
			shaderCursor.WriteField("objects", *renderContext.ObjectBuffer);
			shaderCursor.WriteField("materials", *renderContext.PackedMaterialBuffer);
			shaderCursor.WriteField("textures", draw.Material->GetMaterialInfo().SampledTextures);
			shaderCursor.WriteField("brdfLUT", *renderContext.BRDFLUT);
			shaderCursor.WriteField("irradianceMap", *renderContext.IrradianceCube);
			shaderCursor.WriteField("prefilteredMap", *renderContext.PrefilteredEnvironmentMap);
			shaderCursor.Finalize();
		}

		Renderer::DrawObjects(commandBuffer, mName, renderContext);
	}


}