module ForwardPass;

import Common.Common;
import Common.Material;
import Common.BRDF;
import Common.Lights;

float3 SamplePrefilteredReflection(float3 r, float roughness, SamplerCube prefilteredMap)
{
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	float3 a = prefilteredMap.SampleLevel(r, lodf).rgb;
	float3 b = prefilteredMap.SampleLevel(r, lodc).rgb;
	return lerp(a, b, lod - lodf);
}

struct CoarseVertex
{
	float3 Normal;
	float3 Tangent;
	float2 UV;
	float3 FragPosition;
	nointerpolation uint MaterialHandle;
};

struct VSOutput
{
	float4       Position:     SV_Position;
	CoarseVertex CoarseVertex: CoarseVertex;
};

ConstantBuffer<Scene> gScene;

[shader("vertex")]
VSOutput VSMain(StructuredBuffer<Object> objects, uniform uint instanceOffset, uint instanceID: SV_InstanceID, uint vertexID: SV_VertexID)
{
	Object     object = objects[instanceOffset + instanceID];
	MeshVertex vertex = object.Vertices[vertexID];

	float4 positionLocal = float4(vertex.Position.xyz, 1.0);
	float4 positionWorld = mul(object.Transform, positionLocal);

	VSOutput output;
	output.Position = mul(gScene.Camera.ViewProjection, positionWorld);

	output.CoarseVertex.Normal         = mul(float3x3(object.NormalMatrix), vertex.Normal.xyz);
	output.CoarseVertex.Tangent        = vertex.Tangent.xyz;
	output.CoarseVertex.FragPosition   = positionWorld.xyz;
	output.CoarseVertex.UV             = vertex.UV.xy;
	output.CoarseVertex.MaterialHandle = object.MaterialHandle;

	return output;
}

[shader("fragment")]
float4 FSMain(CoarseVertex coarseVertex, StructuredBuffer<PackedMaterial> materials, Sampler2D textures[], Sampler2D brdfLUT, SamplerCube irradianceMap, SamplerCube prefilteredMap): SV_Target
{
	PackedMaterial packedMaterial = materials[coarseVertex.MaterialHandle];

	Material material = packedMaterial.Unpack(
		textures[packedMaterial.BaseColorMapHandle],
		textures[packedMaterial.NormalMapHandle],
		textures[packedMaterial.OcclusionRoughnessMetallicMapHandle],
		textures[packedMaterial.EmissiveMap],
		coarseVertex.UV
    );

	float3 normal = normalize(coarseVertex.Normal);

	CookTorranceBRDF brdf = material.PrepareBRDF();

	float3 luminance = 0.0;
	float3 viewDir  = normalize(gScene.Camera.Translation - coarseVertex.FragPosition);

	luminance += gScene.DirectionalLight.Illuminate(brdf, normal, viewDir);

	float2 brdfLookUp = brdfLUT.Sample(float2(saturate(dot(normal, viewDir)), material.Roughness)).rg;
	float3 reflection = SamplePrefilteredReflection(reflect(-viewDir, normal), material.Roughness, prefilteredMap).rgb;
	float3 irradiance = irradianceMap.Sample(normal).rgb;

	float3 ambient = brdf.CalculateAmbient(normal, viewDir, brdfLookUp, irradiance, reflection);
	float3 color = luminance + ambient;

	color = color / (color + float3(1.0)); // HDR tonemapping
	color = pow(color, float3(1.0 / 2.2)); // Gamma correction
	
	return float4(color, 1.0);
}

