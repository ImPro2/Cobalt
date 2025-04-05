#pragma once
#include <vulkan/vulkan.h>
#include <initializer_list>
#include <vector>

namespace Cobalt
{

	uint32_t SizeOfVkFormat(VkFormat type);

	struct VertexInputLayoutAttribute
	{
		VertexInputLayoutAttribute() = default;
		VertexInputLayoutAttribute(uint32_t location, VkFormat type, bool normalized = false)
			: Location(location), Type(type), Normalized(normalized), Size(SizeOfVkFormat(type))
		{
		}

		uint32_t Location;
		VkFormat Type;
		bool Normalized;
		uint32_t Size;   // Inferred from Type
		uint32_t Offset = 0; // To be calculated
	};

	struct VertexInputLayout
	{
	public:
		VertexInputLayout(std::initializer_list<VertexInputLayoutAttribute> attributes);

		const std::vector<VertexInputLayoutAttribute>& GetAttributes() const { return mAttributes; }
		uint32_t GetStride() const { return mStride; }

	private:
		void CalculateOffsetsAndStride();

	private:
		std::vector<VertexInputLayoutAttribute> mAttributes;
		uint32_t mStride;
	};

}