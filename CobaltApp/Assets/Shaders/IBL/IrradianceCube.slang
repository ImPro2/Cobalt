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
	float3 LocalPosition;
};

[shader("vertex")]
VSOutput Main(uniform ConstantBuffer<VSInput> vsInput, uint vertexID: SV_VertexID)
{
	VSOutput output;
	output.Position = mul(vsInput.ViewProjections[vsInput.InvocationIndex], vsInput.Vertices[vertexID].Position);
	output.LocalPosition = output.Position.xyz;

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
	float3 irradiance = float3(0.0);
	float sampleCount = 0;

	float3 normal = normalize(input.LocalPosition);
	float3 right  = cross(float3(0.0, 1.0, 0.0), normal);
	float3 up     = cross(normal, right);
	
	const float TWO_PI  = 2.0 * float.getPi();
	const float HALF_PI = 0.5 * float.getPi();
	
	for (float phi = 0.0; phi < TWO_PI; phi += fsInput.deltaPhi)
	{
		for (float theta = 0.0; theta < HALF_PI; theta += fsInput.deltaTheta)
		{
			float3 tangent = right * cos(phi) + up * sin(phi);
			float3 sampleVector = normal * cos(theta) + tangent * sin(theta);
			irradiance += fsInput.environmentMap.Sample(sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount += 1.0;
		}
	}

	return float4(irradiance, 1.0);
}
