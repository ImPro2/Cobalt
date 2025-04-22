#include "Mesh.hpp"

namespace Cobalt
{

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, Material* material)
		: mVertices(vertices), mIndices(indices), mMaterial(material)
	{
		mVertexBuffer = VulkanBuffer::CreateGPUBufferFromCPUData(0, mVertices.size(), mVertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mIndexBuffer  = VulkanBuffer::CreateGPUBufferFromCPUData(0, mIndices.size(), mIndices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	Mesh::~Mesh()
	{
	}

}