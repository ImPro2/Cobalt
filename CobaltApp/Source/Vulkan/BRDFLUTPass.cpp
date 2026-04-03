#include "copch.hpp"
#include "BRDFLUTPass.hpp"
#include "VulkanCommands.hpp"
#include "RenderGraph.hpp"

namespace Cobalt
{

	BRDFLUTPass::BRDFLUTPass()
		: RenderPass("BRDFLUT Pass", "", (RenderPassFlags)RenderPassFlagBits::None)
	{
		CO_PROFILE_FN();
	}

	BRDFLUTPass::~BRDFLUTPass()
	{
		CO_PROFILE_FN();
	}

	void BRDFLUTPass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mTextureHandle = builder.DeclareResource("BRDFLUT", RGResourceInfo {
			.ResourceSizeFlags = RGResourceSizeFlags::Absolute,
			.Format = VK_FORMAT_R16G16_SFLOAT,
			.Transient = false,
			.Sampled = true,
			.Width = mDimensions,
			.Height = mDimensions
		});

		builder.AddDependency(mTextureHandle, RGAccessType::ColorAttachmentWrite);
		builder.SetExecutionCount(1);

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\BRDFLUT.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, mFormat }
			}
		};

		mPipelineBindings = PipelineBindings(GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("", pipelineInfo));
	}
	
	void BRDFLUTPass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		mTexture = mRenderGraph.GetResource(mTextureHandle);

		mRenderGraph.BeginPass(commandBuffer, mPassHandle);
		VulkanCommands::SetViewport(commandBuffer, { mDimensions, mDimensions }, false);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineBindings.PipelineRef->GetPipeline());
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		mRenderGraph.EndPass(commandBuffer, mPassHandle);

		VulkanCommands::TransitionImageLayout(commandBuffer, *mTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

}