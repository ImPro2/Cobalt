import Common.BRDF;

[[SpecializationConstant]] const uint SAMPLE_COUNT = 1024u;

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float Random(float2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,float2(a,b));
	float sn= fmod(dt,3.14);
	return frac(sin(sn) * c);
}

float2 Hammersley2D(uint i, uint N)
{
	// Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10;
	return float2(float(i) /float(N), rdi);
}

// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
float3 ImportanceSample_GGX(float2 Xi, float roughness, float3 normal)
{
	// Maps a 2D point to a hemisphere with spread based on roughness
	float alpha = roughness * roughness;
	float phi = 2.0 * float.getPi() * Xi.x + Random(normal.xz) * 0.1;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha*alpha - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float3 H = float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
	float3 up = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangentX = normalize(cross(up, normal));
	float3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

// Geometric Shadowing function
float G_SchlickSmithGGX(float nDotL, float nDotV, float roughness)
{
	float k = (roughness * roughness) / 2.0;
	float GL = nDotL / (nDotL * (1.0 - k) + k);
	float GV = nDotV / (nDotV * (1.0 - k) + k);
	return GL * GV;
}

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