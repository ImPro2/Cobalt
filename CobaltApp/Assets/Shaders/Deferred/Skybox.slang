import Common.Common;

struct Uniforms
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
	ConstBufferPointer<MeshVertex> MeshVertices;
};

struct VSOutput
{
	float4 Position: SV_Position;
	float3 UVW;
};

[shader("vertex")]
VSOutput VSMain(uniform ConstantBuffer<Uniforms> uniforms, uint vertexID: SV_VertexID)
{
	MeshVertex meshVertex = uniforms.MeshVertices[vertexID];

	// Flip y
	float4x4 proj = uniforms.ProjectionMatrix;
	//proj[1][1] *= -1.0;

	// Remove translation
	float4x4 view = uniforms.ViewMatrix;
	view[0][3] = 0.0;
	view[1][3] = 0.0;
	view[2][3] = 0.0;

	VSOutput output;
	output.Position = mul(proj, mul(view, float4(meshVertex.Position.xyz, 1.0)));
	output.UVW = meshVertex.Position.xyz;

	return output;
}

[shader("fragment")]
float4 FSMain(VSOutput input, uniform SamplerCube skybox)
{
	return skybox.Sample(input.UVW);
}