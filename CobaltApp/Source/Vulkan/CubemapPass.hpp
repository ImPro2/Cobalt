#pragma once
#include "RenderPass.hpp"
#include "Texture.hpp"

namespace Cobalt
{

	class CubemapPass : public RenderPass
	{
	public:
		CubemapPass(const std::string& passName, const std::filesystem::path& shaderPath, RenderPassFlags flags, uint32_t dimensions, VkFormat format);
		virtual ~CubemapPass();

	public:
		Cubemap* GetCubemap() const { return mCubemap.get(); }
		std::unique_ptr<Cubemap> TransferCubemap() { return std::move(mCubemap); }

	public:
		virtual void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

		virtual void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize) = 0;

	protected:
		uint32_t mDimensions;
		VkFormat mFormat;
		uint32_t mMipLevels;

		RGResourceHandle mFramebufferHandle;

		std::vector<glm::mat4> mViewMatrices; // Per cube face

		std::unique_ptr<Cubemap> mCubemap;

	private:
		uint32_t mInvocationIndex = 0;
		uint32_t mInvocationCount = 0;
	};

}
