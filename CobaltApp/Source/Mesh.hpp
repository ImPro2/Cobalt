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
		glm::vec2 TexCoords;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, MaterialHandle materialHandle);
		~Mesh();

	public:
		const std::vector<MeshVertex>& GetVertices() const { return mVertices; }
		const std::vector<uint32_t>& GetIndices() const { return mIndices; }

		const VulkanBuffer& GetVertexBuffer() const { return *mVertexBuffer; }
		const VulkanBuffer& GetIndexBuffer()  const { return *mIndexBuffer;  }

		MaterialHandle GetMaterialHandle() const { return mMaterialHandle; }

	private:
		std::vector<MeshVertex> mVertices;
		std::vector<uint32_t> mIndices;

		MaterialHandle mMaterialHandle;

		std::unique_ptr<VulkanBuffer> mVertexBuffer;
		std::unique_ptr<VulkanBuffer> mIndexBuffer;
	};

}
