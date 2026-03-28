#pragma once
#include "RenderPass.hpp"

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
		uint32_t mDimensions = 64;

		RGResourceHandle mFramebufferHandle = 0;
		Pipeline* mPipeline = nullptr;
		DescriptorHandle mDescriptorHandle = 0;

		Cubemap* mEnvironmentMap = nullptr;
		const Mesh* mMesh = nullptr;

		struct VSInputBuffer
		{
			glm::mat4 ViewProjections[6]; // per invocation
			VkDeviceAddress Vertices;
			uint32_t InvocationIndex;
		};
		
		struct FSInputBuffer
		{
			float deltaPhi;
			float deltaTheta;
		};

		std::unique_ptr<VulkanBuffer> mVSInputBuffer, mFSInputBuffer;

		std::unique_ptr<Cubemap> mIrradianceCube;
	};

}
