#include "InputLayout.hpp"

namespace Cobalt
{

	uint32_t SizeOfVkFormat(VkFormat type)
	{
		switch (type)
		{
			case VK_FORMAT_R32_SFLOAT:          return 1 * sizeof(float);
			case VK_FORMAT_R32G32_SFLOAT:       return 2 * sizeof(float);
			case VK_FORMAT_R32G32B32_SFLOAT:    return 3 * sizeof(float);
			case VK_FORMAT_R32G32B32A32_SFLOAT: return 4 * sizeof(float);
		}

		return 0;
	}


	VertexInputLayout::VertexInputLayout(std::initializer_list<VertexInputLayoutAttribute> attributes)
		: mAttributes(attributes)
	{
		CalculateOffsetsAndStride();
	}

	void VertexInputLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0;
		mStride = 0;

		for (auto& attribute : mAttributes)
		{
			attribute.Offset = offset;

			offset  += attribute.Size;
			mStride += attribute.Size;
		}

	}

}