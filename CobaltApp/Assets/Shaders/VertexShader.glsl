#version 460
#include "Common.glsl"

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vFragPosition;
layout(location = 2) out flat int vBaseInstance;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

layout(std430, set = 0, binding = 0) uniform SceneBuffer
{
	mat4 ViewProjection;
	vec3 CameraPosition;

	DirectionalLightData DirectionalLight;
	PointLightData PointLight;
} uSceneBuffer;

layout(std340, set = 1, binding = 0) readonly buffer ObjectBuffer
{
	ObjectData Objects[];
} uObjectBuffer;

void main()
{
	mat4 transform = uObjectBuffer.Objects[gl_BaseInstance].Transform;
	mat3 normalMatrix = mat3(transpose(inverse(transform)));

	gl_Position = uSceneBuffer.ViewProjection * transform * vec4(aPosition, 1.0);
	vNormal = normalMatrix * aNormal;
	vFragPosition = vec3(transform * vec4(aPosition, 1.0));
	vBaseInstance = gl_BaseInstance;
}