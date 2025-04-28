#include "Material.hpp"
#include "Vulkan/Renderer.hpp"

namespace Cobalt
{

	Material::Material(const Pipeline& pipeline, MaterialData* data)
		: mPipeline(pipeline), mMaterialData(data)
	{
	}

	Material::~Material()
	{
	}

}