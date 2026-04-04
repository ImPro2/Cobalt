import Common.Common;

float2 SampleSphericalMap(float3 v)
{
	const float2 invAtan = float2(0.1591, 0.3183);
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

struct VSOutput
{
	float4 Position: SV_Position;
	float3 LocalPosition;
};

struct Uniforms
{
	float4x4 ViewProjection;
	ConstBufferPointer<MeshVertex> Vertices;
};

[shader("vertex")]
VSOutput Main(uniform Uniforms uniforms, uint vertexID: SV_VertexID)
{
	MeshVertex vertex = uniforms.Vertices[vertexID];

	VSOutput output;
	output.Position = mul(uniforms.ViewProjection, float4(vertex.Position.xyz, 1.0));
	output.LocalPosition = vertex.Position.xyz;

	return output;
}

[shader("fragment")]
float4 Fragment(VSOutput input, Sampler2D equirectangularMap)
{
	float2 uv = SampleSphericalMap(normalize(input.LocalPosition));
	return float4(equirectangularMap.Sample(uv).rgb, 1.0);
}