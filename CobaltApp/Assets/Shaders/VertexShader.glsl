#version 440 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;

layout(push_constant) uniform PushConstants
{
	mat4 ViewProjection;
	mat4 Transform;
} uPushConstants;

layout(location = 0) out vec3 vColor;
layout(location = 1) out flat vec3 vNormal;

void main()
{
	gl_Position = uPushConstants.ViewProjection * uPushConstants.Transform * vec4(aPosition, 1.0);

	mat3 normalMatrix = mat3(transpose(inverse(uPushConstants.Transform)));

	vColor  = aColor;
	vNormal = normalMatrix * aNormal;
}