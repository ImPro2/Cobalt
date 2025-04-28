#include "Mesh.hpp"

namespace Cobalt
{

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, MaterialHandle materialHandle)
		: mVertices(vertices), mIndices(indices), mMaterialHandle(materialHandle)
	{
		mVertexBuffer = VulkanBuffer::CreateGPUBufferFromCPUData(mVertices.data(), sizeof(MeshVertex) * mVertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		mIndexBuffer  = VulkanBuffer::CreateGPUBufferFromCPUData(mIndices.data(), sizeof(uint32_t) * mIndices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	Mesh::~Mesh()
	{
	}

}