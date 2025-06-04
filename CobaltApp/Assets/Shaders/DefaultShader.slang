struct CameraData
{
	float4x4 ViewProjection;
	float3 Translation;
};

struct DirectionalLightData
{
	float3 Direction;

	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
};

struct PointLightData
{
	float3 Position;

	float3 Ambient;
	float3 Diffuse;
	float3 Specular;

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
	uint32_t PointLightCount;
};

struct MeshVertex
{
	float3 Position;
	float TexCoordU;
	float3 Normal;
	float TexCoordV;
};

struct MaterialData
{
	uint32_t DiffuseMapHandle;
	uint32_t SpecularMapHandle;
	float    Shininess;
};

struct VertexBuffer
{
	MeshVertex Vertices[];
};

struct ObjectData
{
	float4x4 Transform;
	float4x4 NormalMatrix;
	ConstBufferPointer<VertexBuffer> VertexBufferRef;
	uint32_t MaterialHandle;
};

struct VSInput
{
	uint32_t InstanceIndex : SV_InstanceID;
	uint32_t VertexIndex : SV_VertexID;
};

struct VSOutput
{
	float4 Position: SV_POSITION;
	
	float3 Normal;
	float2 UV;
	float3 FragPosition;
	nointerpolation uint32_t MaterialHandle;
};

struct ObjectStorageBuffer
{
	ObjectData Objects[];
};

struct MaterialStorageBuffer
{
	MaterialData Materials[];
};

struct UnpackedMaterialData
{
	float3 Diffuse;
	float3 Specular;
	float  Shininess;
};

float3 CalculateDirectionalLightRadiance(DirectionalLightData directionalLight, UnpackedMaterialData material, float3 normal, float3 viewDir)
{
	float3 lightDir = normalize(-directionalLight.Direction);
	float3 reflectDir = reflect(-lightDir, normal);

	float diff = max(0.0, dot(lightDir, normal));
	float spec = pow(max(0.0, dot(viewDir, reflectDir)), material.Shininess);

	float3 ambient = directionalLight.Ambient * material.Diffuse;
	float3 diffuse = directionalLight.Diffuse * diff * material.Diffuse;
	float3 specular =directionalLight.Specular * spec * material.Specular;

	return ambient + diffuse + specular;
}

__generic<typename T> typedef ConstantBuffer<T> UniformBuffer;
__generic<typename T> typedef StructuredBuffer<T, Std430DataLayout> StorageBuffer;

[[vk::binding(0, 0)]] UniformBuffer<SceneData> uScene;
[[vk::binding(1, 0)]] StorageBuffer<ObjectStorageBuffer> sObjectBuffer;
[[vk::binding(2, 0)]] StorageBuffer<MaterialStorageBuffer> sMaterialBuffer;
[[vk::binding(3, 0)]] Sampler2D uTextures[];

[shader("vertex")]
VSOutput VSMain(VSInput input)
{
	ObjectData object = sObjectBuffer[0].Objects[input.InstanceIndex];
	MeshVertex vertex = object.VertexBufferRef.get().Vertices[input.VertexIndex];

	float4 positionLocal = float4(vertex.Position, 1.0);
	float4 positionWorld = mul(object.Transform, positionLocal);

	VSOutput output;
	output.Position = mul(uScene.Camera.ViewProjection, positionWorld);
	output.Normal = mul(float3x3(object.NormalMatrix), vertex.Normal);
	output.UV = float2(vertex.TexCoordU, vertex.TexCoordV);
	output.FragPosition = positionWorld.xyz;
	output.MaterialHandle = object.MaterialHandle;

	return output;
}

[shader("fragment")]
float4 FSMain(VSOutput input) : SV_Target0
{
	MaterialData material = sMaterialBuffer[0].Materials[input.MaterialHandle];

	UnpackedMaterialData unpackedMaterial;
	unpackedMaterial.Diffuse  = uTextures[NonUniformResourceIndex(material.DiffuseMapHandle)].Sample(input.UV).xyz;
	unpackedMaterial.Specular = uTextures[NonUniformResourceIndex(material.SpecularMapHandle)].Sample(input.UV).xyz;
	unpackedMaterial.Shininess = material.Shininess;

	float3 radiance = 0.0;

	float3 normal = normalize(input.Normal);
	float3 viewDir = normalize(uScene.Camera.Translation - input.FragPosition);

	radiance += CalculateDirectionalLightRadiance(uScene.DirectionalLight, unpackedMaterial, normal, viewDir);
	
	return float4(radiance, 1.0);
}

