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
		float TexCoordU;
		glm::vec3 Normal;
		float TexCoordV;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const MaterialData& materialData);
		~Mesh();

	public:
		const std::vector<MeshVertex>& GetVertices() const { return mVertices; }
		const std::vector<uint32_t>& GetIndices() const { return mIndices; }

		VulkanBuffer* GetVertexBuffer() const { return mVertexBuffer.get(); }
		VulkanBuffer* GetIndexBuffer()  const { return mIndexBuffer.get();  }

		VkDeviceAddress GetVertexBufferReference() const { return mVertexBufferReference; }

		Material* GetMaterial() const { return mMaterial.get(); }

	private:
		std::vector<MeshVertex> mVertices;
		std::vector<uint32_t> mIndices;

		std::unique_ptr<VulkanBuffer> mVertexBuffer;
		std::unique_ptr<VulkanBuffer> mIndexBuffer;

		VkDeviceAddress mVertexBufferReference;

		std::shared_ptr<Material> mMaterial;

	};

}
