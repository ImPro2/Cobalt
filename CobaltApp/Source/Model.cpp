#include "Model.hpp"
#include "Vulkan/Renderer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Cobalt
{

	Model::Model(const std::string& modelpath)
	{
		LoadModel(modelpath);
	}

	Model::~Model()
	{

	}

	void Model::LoadModel(const std::string& modelPath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "Assimp Error: Failed to load module " << modelPath << std::endl;
		}

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// Store meshes

		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			mMeshes.push_back(ProcessMesh(mesh, scene));
		}

		// Recurse for each child

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;

		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D position = mesh->mVertices[i];
			aiVector3D normal = mesh->mNormals[i];
			aiVector3D texCoords = { 0.0f, 0.0f, 0.0f };

			if (mesh->HasTextureCoords(i))
				texCoords = mesh->mTextureCoords[i][0];

			vertices.push_back(MeshVertex {
				.Position = { position.x, position.y, position.z },
				.Normal = { normal.x, normal.y, normal.z },
				.TexCoords = { texCoords.x, texCoords.y, texCoords.z }
			});
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			std::vector<uint32_t> faceIndices(face.mIndices, face.mIndices + face.mNumIndices);
			indices.insert(indices.begin(), faceIndices.begin(), faceIndices.end());
		}

		MaterialData materialData;

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			materialData.DiffuseMapHandle  = LoadMaterialTexture(material, aiTextureType_DIFFUSE);
			materialData.SpecularMapHandle = LoadMaterialTexture(material, aiTextureType_SPECULAR);

			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &materialData.Shininess);
		}

		return Mesh(vertices, indices, Renderer::CreateMaterial(materialData));
	}

	TextureHandle Model::LoadMaterialTexture(aiMaterial* material, aiTextureType type)
	{
		static std::unordered_map<aiString, TextureHandle> loadedTexturePaths;

		if (material->GetTextureCount(type) == 0)
			return (TextureHandle)-1;

		aiString path;
		material->GetTexture(type, 0, &path);

		if (loadedTexturePaths.find(path) != loadedTexturePaths.end())
		{
			return loadedTexturePaths[path];
		}
		else
		{
			return Renderer::CreateTexture(TextureInfo(path.C_Str()));
		}
	}

}