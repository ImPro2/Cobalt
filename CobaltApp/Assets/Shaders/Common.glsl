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

struct MaterialData
{
	uint  DiffuseMapHandle;
	uint  SpecularMapHandle;
	float Shininess;
};

struct ObjectData
{
	mat4 Transform;
	mat4 NormalMatrix;
	uint MaterialHandle;
};
