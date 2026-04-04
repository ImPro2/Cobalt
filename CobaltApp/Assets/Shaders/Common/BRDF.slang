module BRDF;

public interface IBRDF
{
	float3 Evaluate(float3 v, float3 l, float3 n);
};

public struct CookTorranceBRDF : IBRDF
{
	public __init(in float3 baseColor, in float roughness, in float metallic)
	{
		BaseColor = baseColor;
		NormalisedBaseColor = baseColor / float.getPi(); // for diffuse normalisation
		Roughness = roughness;
		Metallic = metallic;
		
		F0 = float3(0.04);
		F0 = lerp(F0, baseColor, Metallic);
	}
	
	float3 BaseColor;
	float3 NormalisedBaseColor;
	float Roughness;
	float Metallic;

	float3 F0;

	public float3 Evaluate(in float3 v, in float3 l, in float3 n)
	{
		float3 h = normalize(v + l);

		float nDotL = saturate(dot(n, l));
		float nDotV = saturate(dot(n, v));
		float nDotH = saturate(dot(n, h));
		float hDotV = saturate(dot(h, v));

		float  NDF = DistributionGGX(nDotH, Roughness);
		float  G   = GeometrySmith(nDotV, nDotL, Roughness);
		float3 F   = FresnelSchlick(hDotV, F0);

		float3 kS = F;
		float3 kD = float3(1.0) - kS;
		kD *= 1.0 - Metallic;

		float3 diffuse = kD * NormalisedBaseColor;
		float3 specular = NDF * G * F / (4.0 * nDotV * nDotL + 1e-4);

		return (diffuse + specular) * nDotL;
	}

	public float3 CalculateAmbient(in float3 n, in float3 v, in float2 brdf, in float3 irradiance, in float3 reflection)
	{
		float3 diffuse = irradiance * BaseColor.rgb;

		float3 F = FresnelSchlickRoughness(saturate(dot(n, v)), F0, Roughness);
		float3 specular = reflection * (F * brdf.x + brdf.y);

		float3 kD = float3(1.0) - F;
		kD *= 1.0 - Metallic;

		return kD * diffuse + specular;
	}
};

// Trowbridge-Reitz GGX normal distribution function
public float DistributionGGX(in float nDotH, in float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;

	float num = a2;
	float denom = nDotH * nDotH * (a2 - 1.0) + 1.0;
	denom = denom * denom * float.getPi();

	return num / denom;
}

public float GeometrySchlickGGX(float nDotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	float num   = nDotV;
	float denom = nDotV * (1.0 - k) + k;

	return num / denom;
}

public float GeometrySmith(in float nDotV, in float nDotL, in float roughness)
{
	float ggx1  = GeometrySchlickGGX(nDotV, roughness);
	float ggx2  = GeometrySchlickGGX(nDotL, roughness);

	return ggx1 * ggx2;
}

public float3 FresnelSchlick(in float hDotV, in float3 f0)
{
	float f = pow(saturate(1.0 - hDotV), 5.0);
	//return f + f0 * (1.0 - f);
	return f0 + (float3(1.0) - f0) * f;
}

public float3 FresnelSchlickRoughness(in float cosTheta, in float3 f0, in float roughness)
{
	return f0 + (max(float3(1.0 - roughness), f0) - f0) * pow(saturate(1.0 - cosTheta), 5.0);
}
