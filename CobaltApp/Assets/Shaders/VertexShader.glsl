#version 440 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;

layout(set = 0, binding = 0) uniform SceneData
{
	mat4 ViewProjection;
	vec3 LightPosition;
	vec3 LightColor;
	vec3 CameraPosition;
} uSceneData;

layout(push_constant) uniform PushConstants
{
	mat4 Transform;
} uPushConstants;

layout(location = 0) out vec3 vColor;
layout(location = 1) out flat vec3 vNormal;
layout(location = 2) out vec3 vFragPosition;

void main()
{
	gl_Position = uSceneData.ViewProjection * uPushConstants.Transform * vec4(aPosition, 1.0);

	mat3 normalMatrix = mat3(transpose(inverse(uPushConstants.Transform)));

	vColor  = aColor;
	vNormal = normalMatrix * aNormal;
	vFragPosition = vec3(uPushConstants.Transform * vec4(aPosition, 1.0));
}