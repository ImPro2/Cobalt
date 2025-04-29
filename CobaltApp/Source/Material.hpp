#pragma once
#include "Vulkan/ShaderStructs.hpp"
#include "Vulkan/Pipeline.hpp"

namespace Cobalt
{

	class Material
	{
	public:
		Material(MaterialHandle handle, MaterialData* data);
		~Material();

	public:
		VulkanDescriptorSet* GetGlobalDescriptorSet(uint32_t frameIndex) const { return mGlobalDescriptorSets[frameIndex]; }
		VulkanDescriptorSet* GetMaterialDescriptorSet(uint32_t frameIndex) const { return mMaterialDescriptorSets[frameIndex]; }

	public:
		MaterialHandle GetMaterialHandle() const { return mMaterialHandle; }
		const Pipeline& GetPipeline() const { return *mPipeline; }

		      MaterialData& GetMaterialData()       { return *mMaterialData; }
		const MaterialData& GetMaterialData() const { return *mMaterialData; }

	private:
		MaterialHandle mMaterialHandle;
		std::unique_ptr<Pipeline> mPipeline;
		MaterialData* mMaterialData;

		std::vector<VulkanDescriptorSet*> mGlobalDescriptorSets;
		std::vector<VulkanDescriptorSet*> mMaterialDescriptorSets;
	};

}
