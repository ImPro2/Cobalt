import Common.Common;

struct VSOutput
{
	float4 Position: SV_Position;
	float3 LocalPosition;
};

struct Uniforms
{
	float4x4 ViewProjection;
	ConstBufferPointer<MeshVertex> Vertices;
	float Roughness;
	uint SampleCount;
};

[shader("vertex")]
VSOutput VSMain(uniform Uniforms uniforms, uint vertexID: SV_VertexID)
{
	MeshVertex vertex = uniforms.Vertices[vertexID];

	VSOutput output;
	output.Position = mul(uniforms.ViewProjection, float4(vertex.Position.xyz, 1.0));
	output.LocalPosition = vertex.Position.xyz;

	return output;
}

float3 PrefilterEnvironmentMap(float3 normal, float3 roughness, uint sampleCount)
{
	return float3(0.0);
}

[shader("fragment")]
float4 FSMain(VSOutput input, uniform Uniforms uniforms, SamplerCube environmentMap)
{
	float3 normal = normalize(input.LocalPosition);	
	//return float4(PrefilterEnvironmentMap(normal, uniforms.Roughness, uniforms.SampleCount), 1.0);
	return float4(environmentMap.Sample(normal).xyz, 1.0);
}