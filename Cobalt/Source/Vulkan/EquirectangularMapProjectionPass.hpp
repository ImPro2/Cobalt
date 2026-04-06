#pragma once
#include "CubemapPass.hpp"
#include "PipelineBindings.hpp"

namespace Cobalt
{

	class EquirectangularProjectionPass : public CubemapPass
	{
	public:
		EquirectangularProjectionPass();
		~EquirectangularProjectionPass();

	public:
		void SetEquirectangularMap(const Texture* equirectangularMap) { mEquirectangularMap = equirectangularMap; }
		void SetMesh(const Mesh* mesh)
		{
			mMesh = mesh;
			mCubemap->SetMesh(mesh);
		}

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize) override;

	private:
		PipelineBindings mPipelineBindings;

		const Texture* mEquirectangularMap = nullptr;
		const Mesh* mMesh = nullptr;
	};

}
