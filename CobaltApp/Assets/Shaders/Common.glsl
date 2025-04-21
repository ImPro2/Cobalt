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

struct SceneData
{
	CameraData Camera;
	DirectionalLightData DirectionalLight;
	PointLightData PointLight;
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
	uint MaterialHandle;
};
