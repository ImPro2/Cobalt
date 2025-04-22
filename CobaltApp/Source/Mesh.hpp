#pragma once
#include "Material.hpp"
#include "Vulkan/VulkanBuffer.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace Cobalt
{

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 TexCoords;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, Material* material);
		~Mesh();

	public:
		const VulkanBuffer& GetVertexBuffer() const { return *mVertexBuffer; }
		const VulkanBuffer& GetIndexBuffer()  const { return *mIndexBuffer;  }

	private:
		std::vector<MeshVertex> mVertices;
		std::vector<uint32_t> mIndices;

		Material* mMaterial;

		std::unique_ptr<VulkanBuffer> mVertexBuffer;
		std::unique_ptr<VulkanBuffer> mIndexBuffer;
	};

}
