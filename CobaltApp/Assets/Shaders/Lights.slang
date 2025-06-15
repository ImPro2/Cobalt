module Lights;

import Material;

public struct DirectionalLight
{
	public float3 Direction;

	public float3 Ambient;
	public float3 Diffuse;
	public float3 Specular;

	public float3 CalculateRadiance(UnpackedMaterial material, float3 normal, float3 viewDir)
	{
		float3 lightDir = normalize(-Direction);
		float3 reflectDir = reflect(-lightDir, normal);

		float diff = max(0.0, dot(lightDir, normal));
		float spec = pow(max(0.0, dot(viewDir, reflectDir)), material.Shininess);

		float3 ambient  = Ambient * material.Diffuse;
		float3 diffuse  = Diffuse * diff * material.Diffuse;
		float3 specular = Specular * spec * material.Specular;

		return ambient + diffuse + specular;
	}
};

public struct PointLight
{
	public float3 Position;

	public float3 Ambient;
	public float3 Diffuse;
	public float3 Specular;

	private float Constant;
	private float Linear;
	private float Quadratic;

	public float3 CalculateRadiance()
	{

	}
};