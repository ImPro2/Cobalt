#shader vertex
#version 460
#include "Common.glsl"

layout(location = 0) out      vec3 vNormal;
layout(location = 1) out      vec2 vTexCoord;
layout(location = 2) out      vec3 vFragPosition;
layout(location = 3) out flat uint vMaterialHandle;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	SceneData Scene;
} uSceneData;

layout(set = 2, binding = 0) readonly buffer ObjectDataBlock
{
	ObjectData Objects[];
} uObjectData;

void main()
{
	SceneData scene = uSceneData.Scene;
	ObjectData object = uObjectData.Objects[gl_BaseInstance];

	mat4 transform = object.Transform;
	mat3 normalMatrix = mat3(transpose(inverse(transform)));

	gl_Position = scene.Camera.ViewProjection * transform * vec4(aPosition, 1.0);

	vNormal = normalMatrix * aNormal;
	vTexCoord = aTexCoord;
	vFragPosition = vec3(transform * vec4(aPosition, 1.0));
	vMaterialHandle = object.MaterialHandle;
}

#shader fragment
#version 460
#extension GL_EXT_nonuniform_qualifier : require
#line 44

#include "Common.glsl"

layout(location = 0) out vec4 oColor;

layout(location = 0) in      vec3 vNormal;
layout(location = 1) in      vec2 vTexCoord;
layout(location = 2) in      vec3 vFragPosition;
layout(location = 3) in flat uint vMaterialHandle;

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	SceneData Scene;
} uSceneData;

layout(set = 0, binding = 1) uniform sampler2D uTextures[];

layout(set = 1, binding = 0, std430) readonly buffer MaterialDataBlock
{
	MaterialData Materials[];
} uMaterialData;

void main()
{
	SceneData scene = uSceneData.Scene;
	MaterialData material = uMaterialData.Materials[vMaterialHandle];

	vec3 materialDiffuse  = vec3(texture(uTextures[material.DiffuseMapHandle],  vTexCoord));
	vec3 materialSpecular = vec3(texture(uTextures[material.SpecularMapHandle], vTexCoord));

	vec3 lightDir = normalize(-scene.DirectionalLight.Direction - vFragPosition);
	vec3 viewDir = normalize(scene.Camera.Translation - vFragPosition);
	vec3 reflectDir = reflect(-lightDir, vNormal);

	float diff = max(0.0, dot(lightDir, normalize(vNormal)));
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.Shininess);

	vec3 ambient = scene.DirectionalLight.Ambient * materialDiffuse;
	vec3 diffuse = scene.DirectionalLight.Diffuse * diff * materialDiffuse;
	vec3 specular = scene.DirectionalLight.Specular * spec * materialSpecular;

	oColor = vec4(ambient + diffuse + specular, 1.0);
}