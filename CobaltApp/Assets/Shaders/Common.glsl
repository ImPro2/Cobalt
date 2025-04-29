struct CameraData
{
	mat4 ViewProjection;
	vec3 Translation;
};

struct DirectionalLightData
{
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct PointLightData
{
	vec3 Position;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float Constant;
	float Linear;
	float Quadratic;
};

#define MAX_POINT_LIGHT_COUNT 16

struct SceneData
{
	CameraData Camera;
	DirectionalLightData DirectionalLight;
	PointLightData PointLights[MAX_POINT_LIGHT_COUNT];
	uint PointLightCount;
};

struct MeshVertex
{
	vec3 Position;
	float TexCoordU;
	vec3 Normal;
	float TexCoordV;
};

struct MaterialData
{
	uint  DiffuseMapHandle;
	uint  SpecularMapHandle;
	float Shininess;
};

#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) readonly buffer VertexBuffer
{
	MeshVertex Vertices[];
};

struct ObjectData
{
	mat4 Transform;
	mat4 NormalMatrix;
	VertexBuffer VertexBufferRef;
	uint MaterialHandle;
};

