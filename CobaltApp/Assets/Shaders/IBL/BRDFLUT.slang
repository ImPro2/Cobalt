import IBL.IBLFunctions;

[[SpecializationConstant]] const uint SAMPLE_COUNT = 1024u;


float2 CalculateBRDF(float NoV, float roughness)
{
	const float3 n = float3(0.0, 0.0, 1.0);
	float3 v = float3(sqrt(1.0 - NoV * NoV), 0.0, NoV);

	float2 LUT = float2(0.0);

	for (uint i = 0; i < SAMPLE_COUNT; i++)
	{
		float2 Xi = Hammersley2D(i, SAMPLE_COUNT);
		float3 h = ImportanceSample_GGX(Xi, roughness, n);
		float3 l = 2.0 * dot(v, h) * h - v;

		float nDotL = saturate(dot(n, l));
		float vDotH = saturate(dot(v, h));
		float nDotH = saturate(dot(n, h));
		float nDotV = saturate(dot(n, v));

		if (nDotL > 0.0)
		{
			float G = G_SchlickSmithGGX(nDotL, nDotV, roughness);
			float G_Vis = (G * vDotH) / (nDotH * nDotV);
			float Fc = pow(1.0 - vDotH, 5.0);
			LUT += float2((1.0 - Fc) * G_Vis, Fc * G_Vis);
		}
	}

	return LUT / float(SAMPLE_COUNT);
}

struct VSOutput
{
	float4 Position: SV_Position;
	float2 UV;
};

[shader("vertex")]
VSOutput VSMain(uint vertexID: SV_VertexID)
{
	VSOutput output;
	output.UV       = float2((vertexID << 1) & 2, vertexID & 2);
	output.Position = float4(output.UV * 2.0 - 1.0, 0.0, 1.0);

	return output;
}

[shader("fragment")]
float4 FSMain(VSOutput input)
{
	return float4(CalculateBRDF(input.UV.x, input.UV.y), 0.0, 1.0);
	//return float4(0.0, 0.0, 0.0, 1.0);
}