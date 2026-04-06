#pragma once
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"
#include "ShaderCursor.hpp"

#include <vector>

namespace Cobalt
{

	class GeometryPass : public RenderPass
	{
	public:
		GeometryPass();
		~GeometryPass();

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		RGResourceHandle mPositionAttachment, mBaseColorAttachment, mNormalAttachment, mOCRAttachment, mEmissiveAttachment;
		RGResourceHandle mDepthStencilAttachment;
	};

}
