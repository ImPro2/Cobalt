#pragma once
#include "RenderPass.hpp"
#include "PipelineBindings.hpp"

namespace Cobalt
{

	class BRDFLUTPass : public RenderPass
	{
	public:
		BRDFLUTPass();
		~BRDFLUTPass();

	public:
		Texture* GetBRDFLUT() const { return mTexture; }

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		const uint32_t mDimensions = 512;
		const VkFormat mFormat = VK_FORMAT_R16G16_SFLOAT;

		RGResourceHandle mTextureHandle;
		PipelineBindings mPipelineBindings;
		Texture* mTexture = nullptr;
	};

}
