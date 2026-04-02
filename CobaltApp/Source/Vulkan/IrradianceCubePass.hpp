#pragma once
#include "RenderPass.hpp"
#include "PipelineBindings.hpp"

namespace Cobalt
{

	class IrradianceCubePass : public RenderPass
	{
	public:
		IrradianceCubePass();
		~IrradianceCubePass();

	public:
		void SetEnvironmentMap(Cubemap* envMap, const Mesh* mesh);

		Cubemap* GetIrradianceCube() const { return mIrradianceCube.get(); }

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		uint32_t mDimensions = 32;

		RGResourceHandle mFramebufferHandle = 0;
		PipelineBindings mPipelineBindings;

		Cubemap* mEnvironmentMap = nullptr;
		const Mesh* mMesh = nullptr;

		std::unique_ptr<Cubemap> mIrradianceCube;

		uint32_t mInvocationIndex = 0;
		uint32_t mInvocationCount = 0;
	};

}
