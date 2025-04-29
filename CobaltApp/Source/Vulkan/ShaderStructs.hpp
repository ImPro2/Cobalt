#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#define CO_MAX_POINT_LIGHT_COUNT 16

namespace Cobalt
{

	struct CameraData
	{
		glm::mat4 ViewProjectionMatrix;
		alignas(16) glm::vec3 CameraTranslation;
	};

	struct DirectionalLightData
	{
		alignas(16) glm::vec3 Direction;

		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;
	};

	struct PointLightData
	{
		alignas(16) glm::vec3 Position;

		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;

		float Constant;
		float Linear;
		float Quadratic;

		float __padding0;
	};

	struct SceneData
	{
		CameraData Camera;
		DirectionalLightData DirectionalLight;
		PointLightData PointLights[CO_MAX_POINT_LIGHT_COUNT];
		uint32_t PointLightCount;
	};

	using TextureHandle = uint32_t;
	using MaterialHandle = uint32_t;

	struct MaterialData
	{
		TextureHandle DiffuseMapHandle;
		TextureHandle SpecularMapHandle;
		float Shininess;

		float __padding0;
	};

	struct ObjectData
	{
		glm::mat4 Transform;
		glm::mat4 NormalMatrix;
		alignas(16) VkDeviceAddress VertexBufferRef;
		alignas(16) MaterialHandle MaterialHandle;
	};

}
