#pragma once
#include "RenderPass.hpp"
#include "PipelineBindings.hpp"
#include "CubemapPass.hpp"

namespace Cobalt
{

	class IrradianceCubePass : public CubemapPass
	{
	public:
		IrradianceCubePass();
		~IrradianceCubePass();

	public:
		void SetEnvironmentMap(Cubemap* envMap, const Mesh* mesh);

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize) override;

	private:
		PipelineBindings mPipelineBindings;

		Cubemap* mEnvironmentMap = nullptr;
		const Mesh* mMesh = nullptr;
	};

}
