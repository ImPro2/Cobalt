import Common.Common;

struct VSInput
{
	float4x4 ViewProjection;
	ConstBufferPointer<MeshVertex> Vertices;
};

struct VSOutput
{
	float4 Position: SV_Position;
	float3 LocalPosition;
};

[shader("vertex")]
VSOutput Main(uniform ConstantBuffer<VSInput> vsInput, uint vertexID: SV_VertexID)
{
	MeshVertex vertex = vsInput.Vertices[vertexID];

	VSOutput output;
	output.Position = mul(vsInput.ViewProjection, float4(vertex.Position.xyz, 1.0));
	output.LocalPosition = vertex.Position.xyz;

	return output;
}

struct FSInput
{
	float deltaTheta;
	float deltaPhi;
	SamplerCube environmentMap;
};

[shader("fragment")]
float4 Fragment(VSOutput input, uniform ConstantBuffer<FSInput> fsInput)
{
#if 0
	float3 irradiance = float3(0.0);
	uint sampleCount = 0;

	float3 normal = normalize(input.LocalPosition.xyz);
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right  = normalize(cross(up, normal));
	up = cross(normal, right);
	
	const float TWO_PI  = 2.0 * float.getPi();
	const float HALF_PI = 0.5 * float.getPi();
	
	for (float phi = 0.0; phi < TWO_PI; phi += fsInput.deltaPhi)
	{
		for (float theta = 0.0; theta < HALF_PI; theta += fsInput.deltaTheta)
		{
			float3 tangent = right * cos(phi) + up * sin(phi);
			float3 sampleVector = normal * cos(theta) + tangent * sin(theta);
			irradiance += fsInput.environmentMap.Sample(sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}

	return float4(float.getPi() * irradiance / float(sampleCount), 1.0);
#else
	return float4(fsInput.environmentMap.Sample(input.LocalPosition.xyz).xyz, 1.0);
	//return float4(float(input.instance) / 6.0, 0.0, 0.0, 1.0);
#endif
}
