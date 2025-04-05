#version 440 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
//layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec3 vColor;

void main()
{
	//gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
	gl_Position = vec4(aPosition, 1.0);

	vColor = aColor;
}