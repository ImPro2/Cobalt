#pragma once
#include "Mesh.hpp"
#include <assimp/scene.h>

namespace Cobalt
{

	class Model
	{
	public:
		Model(const std::string& modelpath);
		~Model();

	private:
		void LoadModel(const std::string& modelPath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		TextureHandle LoadMaterialTexture(aiMaterial* material, aiTextureType type);

	private:
		std::vector<Mesh> mMeshes;
	};

}
