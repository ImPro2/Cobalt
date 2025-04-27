#pragma once
#include "Vulkan/Pipeline.hpp"

namespace Cobalt
{

	using TextureHandle = uint32_t;
	using MaterialHandle = uint32_t;

	struct MaterialData
	{
		TextureHandle DiffuseMapHandle;
		TextureHandle SpecularMapHandle;
		float Shininess;
	};

	class Material
	{
	public:
		Material(const std::string& shaderFilePath, MaterialData* data);
		~Material();

	public:
		const Pipeline& GetPipeline() const { return *mPipeline; }

		      MaterialData& GetMaterialData()       { return *mMaterialData; }
		const MaterialData& GetMaterialData() const { return *mMaterialData; }

	private:
		std::unique_ptr<Pipeline> mPipeline;

		MaterialData* mMaterialData;
	};

}
