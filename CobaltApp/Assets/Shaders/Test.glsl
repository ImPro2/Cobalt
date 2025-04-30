#shader vertex
#version 460
#extension GL_EXT_buffer_reference_uvec2 : require

#include "Common.glsl"

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out flat uint vMaterialHandle;
layout(location = 2) out vec3 vNormal;

layout(set = 0, binding = 0) uniform SceneDataBlock
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
	ObjectData object = uObjectData.Objects[gl_BaseInstance];
	VertexBuffer vertexBufferRef = VertexBuffer(object.VertexBufferRef);

	MeshVertex vertex = vertexBufferRef.Vertices[gl_VertexIndex];

	gl_Position = uSceneData.Scene.Camera.ViewProjection *
		object.Transform *
		vec4(vertex.Position, 1.0);

	vTexCoord = vec2(vertex.TexCoordU, vertex.TexCoordV);
	vMaterialHandle = object.MaterialHandle;
	vNormal = vertex.Normal;
}

#shader fragment
#version 460

#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in flat uint vMaterialHandle;
layout(location = 2) in vec3 vNormal;

void main()
{
	oColor = vec4(vNormal, 1.0);
}
