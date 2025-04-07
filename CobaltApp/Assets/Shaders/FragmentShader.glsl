#version 460

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vFragPosition;
layout(location = 2) in flat int vBaseInstance;

struct MaterialData
{
	vec3  Ambient;
	vec3  Diffuse;
	vec3  Specular;
	float Shininess;
};

struct ObjectData
{
	mat4 Transform;
	MaterialData Material;
};

layout(std140, set = 0, binding = 0) uniform SceneBuffer
{
	mat4 ViewProjection;
	vec3 CameraPosition;
	vec3 LightPosition;
	vec3 LightColor;
} uSceneBuffer;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData Objects[];
} uObjectBuffer;

void main()
{
	MaterialData material = uObjectBuffer.Objects[vBaseInstance].Material;

	// Ambient

	vec3 ambient = uSceneBuffer.LightColor * material.Ambient;

	// Diffuse

	vec3 lightDir = normalize(uSceneBuffer.LightPosition - vFragPosition);
	vec3 diffuse = max(0.0, dot(lightDir, normalize(vNormal))) * (uSceneBuffer.LightColor * material.Diffuse);

	// Specular

	vec3 viewDir = normalize(uSceneBuffer.CameraPosition - vFragPosition);
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);
	vec3 specular = uSceneBuffer.LightColor * (spec * material.Specular);

	vec3 result = ambient + diffuse + specular;

	oColor = vec4(result, 1.0);
}