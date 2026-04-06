#pragma once
#include "CubemapPass.hpp"
#include "PipelineBindings.hpp"

namespace Cobalt
{

	class PrefilteredEnvironmentMapPass : public CubemapPass
	{
	public:
		PrefilteredEnvironmentMapPass();
		~PrefilteredEnvironmentMapPass();

	public:
		void SetEnvironmentMap(const Cubemap* environmentMap);

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext, uint32_t face, uint32_t mipLevel, uint32_t viewportSize);

	private:
		PipelineBindings mPipelineBindings;

		const Cubemap* mEnvironmentMap = nullptr;
		const Mesh* mMesh = nullptr;
	};

}
