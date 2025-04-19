#version 460
#include "Common.glsl"

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPosition;
layout(location = 2) in flat int vBaseInstance;

int MAX_LIGHT_COUNT = 16;

layout(std140, set = 0, binding = 0) uniform SceneBuffer
{
	mat4 ViewProjection;
	vec3 CameraPosition;
	
	DirectionalLightData DirectionalLight;
	PointLightData PointLight;
} uSceneBuffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData Objects[];
} uObjectBuffer;

void main()
{
	MaterialData material = uObjectBuffer.Objects[vBaseInstance].Material;

	vec3 lightDir = normalize(-uSceneBuffer.DirectionalLight.Direction - vFragPosition);
	vec3 viewDir = normalize(uSceneBuffer.CameraPosition - vFragPosition);
	vec3 reflectDir = reflect(-lightDir, vNormal);

	float diff = max(0.0, dot(lightDir, normalize(vNormal)));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);

	vec3 ambient = uSceneBuffer.DirectionalLight.Ambient * material.Ambient;
	vec3 diffuse = uSceneBuffer.DirectionalLight.Diffuse * (diff * material.Diffuse);
	vec3 specular = uSceneBuffer.DirectionalLight.Specular * (spec * material.Specular);

	vec3 result = ambient + diffuse + specular;

	oColor = vec4(result, 1.0);
}
