import IBL.IBLFunctions;
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

[shader("fragment")]
float4 FSMain(VSOutput input, uniform Uniforms uniforms, SamplerCube environmentMap)
{
	float3 n = normalize(input.LocalPosition);
	float3 v = n;

	float3 color = float3(0.0);
	float totalWeight = 0.0;

	int2 envMapDims;
	environmentMap.GetDimensions(envMapDims.x, envMapDims.y);

	float envMapDim = float(envMapDims.x);

	for (uint i = 0; i < uniforms.SampleCount; i++)
	{
		float2 Xi = Hammersley2D(i, uniforms.SampleCount);
		float3 h = ImportanceSample_GGX(Xi, uniforms.Roughness, n);
		float3 l = 2.0 * dot(v, h) * h - v;
		float nDotL = saturate(dot(n, l));

		if (nDotL > 0.0)
		{
			float nDotH = saturate(dot(n, h));
			float vDotH = saturate(dot(v, h));

			float pdf = D_GGX(nDotH, uniforms.Roughness) * nDotH / (4.0 * vDotH) + 0.0001;
			float omegaS = 1.0 / (float(uniforms.SampleCount) * pdf);
			float omegaP = 4.0 * float.getPi() / (6.0 * envMapDim * envMapDim);
			float mipLevel = uniforms.Roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0);
			color += environmentMap.SampleLevel(l, mipLevel).rgb * nDotL;
			totalWeight += nDotL;
		}
	}

	return float4(color / totalWeight, 1.0);
	//return float4(1.0, 0.0, 0.0, 1.0);
	//return float4(environmentMap.Sample(normal).xyz, 1.0);
}