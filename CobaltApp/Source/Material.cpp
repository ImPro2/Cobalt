#include "Material.hpp"

namespace Cobalt
{

	Material::Material(const std::string& shaderFilePath, MaterialData* data)
	{
		PipelineInfo pipelineInfo = {
			.Shader = std::make_unique<Shader>(shaderFilePath),
			.PrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.EnableDepthTesting = true
		};

		mPipeline = std::make_unique<Pipeline>(pipelineInfo);
		mMaterialData = data;
	}

	Material::~Material()
	{
	}

}