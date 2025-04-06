#version 440 core

layout(location = 0) in vec3 vColor;
layout(location = 1) in flat vec3 vNormal;
layout(location = 2) in vec3 vFragPosition;

layout(set = 0, binding = 0) uniform SceneData
{
	mat4 ViewProjection;
	vec3 LightPosition;
	vec3 LightColor;
	vec3 CameraPosition;
} uSceneData;

layout(location = 0) out vec4 oColor;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * uSceneData.LightColor;

	vec3 lightDir = normalize(uSceneData.LightPosition - vFragPosition);
	vec3 diffuse = max(0.0, dot(lightDir, normalize(vNormal))) * uSceneData.LightColor;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(uSceneData.CameraPosition - vFragPosition);
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * spec * uSceneData.LightColor;

	vec3 result = (ambient + diffuse + specular) * vColor;

	oColor = vec4(result, 1.0);
}