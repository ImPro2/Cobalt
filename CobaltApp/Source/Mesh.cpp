#include "Mesh.hpp"
#include "Vulkan/GraphicsContext.hpp"
#include "Vulkan/Renderer.hpp"

namespace Cobalt
{

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const MaterialData& materialData)
		: mVertices(vertices), mIndices(indices)
	{
		mVertexBuffer = VulkanBuffer::CreateGPUBufferFromCPUData(mVertices.data(), sizeof(MeshVertex) * mVertices.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
		mIndexBuffer  = VulkanBuffer::CreateGPUBufferFromCPUData(mIndices.data(), sizeof(uint32_t) * mIndices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		mVertexBufferReference = mVertexBuffer->GetDeviceAddress();

		mMaterial = Renderer::CreateMaterial(materialData);
	}

	Mesh::~Mesh()
	{
	}

}