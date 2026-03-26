#include "copch.hpp"
#include "IrradianceCubePass.hpp"
#include "GraphicsContext.hpp"
#include "VulkanCommands.hpp"

namespace Cobalt
{

	IrradianceCubePass::IrradianceCubePass(Cubemap* cubemap, const Mesh* cubemapMesh)
		: RenderPass("Irradiance Cube Pass", "", (RenderPassFlags)RenderPassFlagBits::SideAffect),
		mCubemap(cubemap), mCubemapMesh(cubemapMesh)
	{
		CO_PROFILE_FN();
	}

	IrradianceCubePass::~IrradianceCubePass()
	{
		CO_PROFILE_FN();
	}

	void IrradianceCubePass::Setup(RenderGraphBuilder& builder)
	{
		CO_PROFILE_FN();

		mFramebuffer = std::make_unique<Texture>(
			TextureInfo(
				mCubemap->GetWidth(), mCubemap->GetHeight(), 
				VK_FORMAT_R32G32B32A32_SFLOAT,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			)
		);

		GraphicsContext::Get().SubmitSingleTimeCommands(GraphicsContext::Get().GetQueue(), [&](VkCommandBuffer commandBuffer)
		{
			VulkanCommands::TransitionImageLayout(commandBuffer, *mFramebuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			VulkanCommands::TransitionImageLayout(commandBuffer, *mCubemap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		});

		builder.SetExecutionCount(6);
		builder.SetExternalResource(mFramebuffer.get());

		PipelineInfo pipelineInfo = {
			.Shader = Renderer::GetShaderLibrary().GetShader("IBL\\IrradianceCube.slang"),
			.EnableDepthTesting = false,
			.ColorAttachments = {
				{ false, VK_FORMAT_R32G32B32A32_SFLOAT }
			}
		};

		mPipeline = GraphicsContext::Get().GetPipelineRegistry().BuildPipeline("Irradiance Cube Pipeline", pipelineInfo);

		VkDescriptorSetLayout descriptorSetLayout = pipelineInfo.Shader->GetDescriptorSetLayouts()[0];
		mDescriptorHandle = GraphicsContext::Get().GetDescriptorBufferManager().AllocateDescriptor(descriptorSetLayout, true, true);

		mVSInputBuffer = VulkanBuffer::CreateMappedBuffer(sizeof(VSInputBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		mFSInputBuffer = VulkanBuffer::CreateMappedBuffer(sizeof(FSInputBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	}

	void IrradianceCubePass::Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext)
	{
		CO_PROFILE_FN();

		static uint32_t invocationIndex = 0;

		VSInputBuffer vsInput;
		vsInput.ViewProjections[invocationIndex] = glm::mat4(1.0f);
		vsInput.Vertices = mCubemapMesh->GetVertexBufferReference();
		vsInput.InvocationIndex = invocationIndex++;

		FSInputBuffer fsInput;
		fsInput.deltaPhi = 0;
		fsInput.deltaTheta = 0;

		mVSInputBuffer->CopyData(&vsInput);
		mFSInputBuffer->CopyData(&fsInput);

		VulkanCommands::SetViewport(commandBuffer, { .width = mCubemap->GetWidth(), .height = mCubemap->GetHeight() });

		ShaderCursor shaderCursor(mPipeline->GetInfo().Shader->GetRootShaderParameter(), mDescriptorHandle);
		shaderCursor.WriteField("vsInputs", *mVSInputBuffer);
		shaderCursor.WriteField("fsInputs", *mFSInputBuffer);
		shaderCursor.Field("fsInputs").WriteField("environmentMap", *mCubemap);
		shaderCursor.Finalize();

		GraphicsContext::Get().GetDescriptorBufferManager().SetDescriptorBufferOffsets(commandBuffer, mPipeline->GetPipelineLayout(), mDescriptorHandle);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->GetPipeline());
		vkCmdBindIndexBuffer(commandBuffer, mCubemapMesh->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, mCubemapMesh->GetIndices().size(), 1, 0, 0, 0);

		VulkanCommands::TransitionImageLayout(commandBuffer, *mFramebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanCommands::CopyImageToCubemapFace(commandBuffer, *mFramebuffer, *mCubemap, invocationIndex);
		VulkanCommands::TransitionImageLayout(commandBuffer, *mFramebuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

}