#pragma once
#include "Mesh.hpp"
#include <assimp/scene.h>
#include <memory>

namespace Cobalt
{

	class Model
	{
	public:
		Model(const std::string& modelpath);
		~Model();

	public:
		const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return mMeshes; }

	private:
		void LoadModel(const std::string& modelPath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
		TextureHandle LoadMaterialTexture(aiMaterial* material, aiTextureType type);

	private:
		std::vector<std::unique_ptr<Mesh>> mMeshes;
	};

}
