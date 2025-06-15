module Material;

public struct UnpackedMaterial
{
	public float3 Diffuse;
	public float3 Specular;
	public float  Shininess;
};

public struct Material
{
	public uint  DiffuseMapHandle;
	public uint  SpecularMapHandle;
	public float Shininess;

	public UnpackedMaterial Unpack(in Sampler2D diffuseTexture, in Sampler2D specularTexture, in float2 uv)
	{
		UnpackedMaterial unpackedMaterial;
		unpackedMaterial.Diffuse  = diffuseTexture.Sample(uv).xyz;
		unpackedMaterial.Specular = specularTexture.Sample(uv).xyz;
		unpackedMaterial.Shininess = Shininess;

		return unpackedMaterial;
	}
};