import Common.Common;

struct VSInput
{
	float4x4 ViewProjections[6];
	ConstBufferPointer<MeshVertex> Vertices;
	uint InvocationIndex;
};

struct VSOutput
{
	float4 Position: SV_Position;
	float3 UVW;
};

[shader("vertex")]
VSOutput Main(uniform VSInput vsInput, uint vertexID: SV_VertexID)
{
	VSOutput output;
	output.Position = mul(input.ViewProjections[input.InvocationIndex], input.Vertices[vertexID].Position);
	output.UVW = output.Position.xyz;

	return output;
}

struct FSInput
{
	float deltaTheta;
	float deltaPhi;
};

[shader("fragment")]
float4 Fragment(VSOutput input, uniform FSInupt fsInput)
{
	return input.UVW;
}
