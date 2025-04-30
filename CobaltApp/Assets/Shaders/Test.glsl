#shader vertex
#version 450
#extension GL_EXT_buffer_reference : require

#include "Common.glsl"

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	SceneData Scene;
} uSceneData;

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	MeshVertex Vertices[];
};

layout(push_constant) uniform PushConstants
{
	mat4 Transform;
	VertexBuffer VertexBufferRef;
} uPushConstants;

void main()
{
	gl_Position = uSceneData.Scene.Camera.ViewProjection *
		uPushConstants.Transform *
		vec4(uPushConstants.VertexBufferRef.Vertices[gl_VertexIndex].Position, 1.0);
}

#shader fragment
#version 450

#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(1.0);
}
