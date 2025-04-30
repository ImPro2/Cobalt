#shader vertex
#version 460
#include "Common.glsl"
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(location = 0) out      vec3 vNormal;
layout(location = 1) out      vec2 vTexCoord;
layout(location = 2) out      vec3 vFragPosition;
layout(location = 3) out flat uint vMaterialHandle;

layout(set = 0, binding = 0, std430) uniform SceneBlock
{
	SceneData Scene;
} uSceneData;

layout(set = 0, binding = 1, std430) readonly buffer ObjectDataBlock
{
	ObjectData Objects[];
} uObjectData;

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	MeshVertex Vertices[];
};

void main()
{
	SceneData scene = uSceneData.Scene;
	ObjectData object = uObjectData.Objects[gl_BaseInstance];

	MeshVertex vertex = VertexBuffer(object.VertexBufferRef).Vertices[gl_VertexIndex];

	vec4 positionLocal = vec4(vertex.Position, 1.0);
	vec4 positionWorld = object.Transform * positionLocal;

	gl_Position = scene.Camera.ViewProjection * positionWorld;

	vNormal = mat3(object.NormalMatrix) * vertex.Normal;
	vTexCoord = vec2(vertex.TexCoordU, vertex.TexCoordV);
	vFragPosition = vec3(positionWorld);
	vMaterialHandle = object.MaterialHandle;
}

#shader fragment
#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
#line 44

#include "Common.glsl"

layout(location = 0) out vec4 oColor;

layout(location = 0) in      vec3 vNormal;
layout(location = 1) in      vec2 vTexCoord;
layout(location = 2) in      vec3 vFragPosition;
layout(location = 3) in flat uint vMaterialHandle;

layout(set = 0, binding = 0, std430) uniform SceneDataBlock
{
	SceneData Scene;
} uSceneData;

layout(set = 0, binding = 2, std430) readonly buffer MaterialDataBlock
{
	MaterialData Materials[];
} uMaterialData;

layout(set = 0, binding = 3) uniform sampler2D uTextures[];

struct UnpackedMaterialData
{
	vec3 Diffuse;
	vec3 Specular;
	float Shininess;
};

vec3 CalculateDirectionalLightRadiance(DirectionalLightData directionalLight, UnpackedMaterialData material, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-directionalLight.Direction);
	vec3 reflectDir = reflect(-lightDir, normal);

	float diff = max(0.0, dot(lightDir, normal));
	float spec = pow(max(0.0, dot(viewDir, reflectDir)), material.Shininess);

	vec3 ambient = directionalLight.Ambient * material.Diffuse;
	vec3 diffuse = directionalLight.Diffuse * diff * material.Diffuse;
	vec3 specular = directionalLight.Specular * spec * material.Specular;

	return ambient + diffuse + specular;
}

vec3 CalculatePointLightRadiance(PointLightData pointLight, UnpackedMaterialData material, vec3 normal, vec3 fragPosition, vec3 viewDir)
{
	vec3 lightDir = normalize(pointLight.Position - fragPosition);
	vec3 reflectDir = reflect(-lightDir, normal);

	float diff = max(0.0, dot(lightDir, normal));
	float spec = pow(max(0.0, dot(viewDir, reflectDir)), material.Shininess);
	float dist = length(pointLight.Position - fragPosition);
	float attenuation = 1.0 / (pointLight.Constant + pointLight.Linear * dist + pointLight.Quadratic * dist * dist);

	vec3 ambient = pointLight.Ambient * material.Diffuse;
	vec3 diffuse = pointLight.Diffuse * diff * material.Diffuse;
	vec3 specular = pointLight.Specular * spec * material.Specular;

	return attenuation * (ambient + diffuse + specular);
}

void main()
{
	SceneData scene = uSceneData.Scene;
	MaterialData material = uMaterialData.Materials[vMaterialHandle];

	UnpackedMaterialData unpackedMaterial;
	unpackedMaterial.Diffuse  = vec3(texture(uTextures[0],  vTexCoord));
	unpackedMaterial.Specular = vec3(texture(uTextures[0], vTexCoord));
	unpackedMaterial.Shininess = material.Shininess;

	vec3 radiance = vec3(0.0);

	vec3 normal = normalize(vNormal);
	vec3 viewDir = normalize(scene.Camera.Translation - vFragPosition);

	radiance += CalculateDirectionalLightRadiance(scene.DirectionalLight, unpackedMaterial, normal, viewDir);

	for (uint i = 0; i < scene.PointLightCount; i++)
		radiance += CalculatePointLightRadiance(scene.PointLights[i], unpackedMaterial, normal, vFragPosition, viewDir);

	oColor = vec4(radiance, 1.0);
	//oColor = vec4(normal, 1.0);
}