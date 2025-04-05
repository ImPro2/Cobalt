#version 440 core

layout(location = 0) in vec3 vColor;
layout(location = 1) in flat vec3 vNormal;

layout(location = 0) out vec4 oColor;

void main()
{
	vec3 lightDir = normalize(vec3(0, 0, 1));

	float lightIntensity = max(0.1, dot(lightDir, vNormal));

	oColor = vec4(lightIntensity * vColor, 1.0);
}