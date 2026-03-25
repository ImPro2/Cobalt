#pragma once
#include "RenderPass.hpp"

namespace Cobalt
{

	class IrradianceCubePass : public RenderPass
	{
	public:
		IrradianceCubePass(Cubemap* cubemap, const Mesh* cubemapMesh);
		~IrradianceCubePass();

	public:
		void Setup(RenderGraphBuilder& builder) override;
		void Execute(VkCommandBuffer commandBuffer, const RenderContext& renderContext) override;

	private:
		Cubemap* mCubemap;
		const Mesh* mCubemapMesh;

		std::unique_ptr<Texture> mFramebuffer;
		Pipeline* mPipeline = nullptr;
		DescriptorHandle mDescriptorHandle;

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
	};

}
