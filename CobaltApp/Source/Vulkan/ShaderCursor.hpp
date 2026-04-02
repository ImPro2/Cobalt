#pragma once
#include "Texture.hpp"
#include "VulkanBuffer.hpp"
#include "ShaderParameters.hpp"
#include "DescriptorBindings.hpp"
#include "DescriptorBufferManager.hpp"

#include <string>
#include <cassert>

namespace Cobalt
{

	class ShaderCursor
	{
	public:
		ShaderCursor(ShaderParameter& shaderParameter, const DescriptorHandle descriptorHandle, const std::vector<VkPushConstantRange>& pushConstantRanges = {}, uint8_t* pushConstantBuffer = nullptr);
		ShaderCursor(const ShaderCursor& other);
		~ShaderCursor();

	private:
		ShaderCursor(ShaderParameter& shaderParameter, DescriptorBindings& descriptorBindings, DescriptorHandle descriptorHandle, const std::vector<VkPushConstantRange>& pushConstantRanges, uint8_t* pushConstantBuffer);

	public:
		void Write(const void* data, size_t size);
		void Write(const VulkanBuffer& buffer);
		void Write(const Texture& texture);
		void Write(const Cubemap& cubemap);
		void Write(const Image& image);
		void Write(const std::vector<Image>& images);

		template<typename T>
		void Write(const T& data)
		{
			Write(&data, sizeof(T));
		}

		ShaderCursor Field(const std::string& name) const
		{
			assert(mShaderParameter.Fields.contains(name));
			return ShaderCursor(mShaderParameter.Fields.at(name), mDescriptorBindingsRef, mDescriptorHandle, mPushConstantRanges, mPushConstantBuffer);
		}

		ShaderCursor Element(uint32_t index) const
		{
			assert(mShaderParameter.Kind == ShaderParameterKind::Array);

			if (index >= mShaderParameter.Elements.size())
			{
				mShaderParameter.Elements.push_back(ShaderParameter{
					.Kind = mShaderParameter.ElementKind,
					.Binding = mShaderParameter.Binding,
					.Index = index,
				});
			}

			return ShaderCursor(mShaderParameter.Elements[index], mDescriptorBindingsRef, mDescriptorHandle, mPushConstantRanges, mPushConstantBuffer);
		}

		ShaderCursor WriteField(const std::string& name, const void* data, size_t size) const
		{
			Field(name).Write(data, size);
			return *this;
		}

		template<typename T>
		ShaderCursor WriteField(const std::string& name, const T& data) const
		{
			return WriteField(name, &data, sizeof(data));
		}

		template<>
		ShaderCursor WriteField<VulkanBuffer>(const std::string& name, const VulkanBuffer& buffer) const
		{
			Field(name).Write(buffer);
			return *this;
		}

		template<>
		ShaderCursor WriteField<Texture>(const std::string& name, const Texture& texture) const
		{
			Field(name).Write(texture);
			return *this;
		}

		template<>
		ShaderCursor WriteField<Cubemap>(const std::string& name, const Cubemap& cubemap) const
		{
			Field(name).Write(cubemap);
			return *this;
		}

		template<>
		ShaderCursor WriteField<std::vector<Image>>(const std::string& name, const std::vector<Image>& images) const
		{
			Field(name).Write(images);
			return *this;
		}

		// Updates descriptor bindings if needed
		void Finalize();

	private:
		ShaderParameter& mShaderParameter;

		DescriptorBindings mDescriptorBindings;
		DescriptorBindings& mDescriptorBindingsRef;

		DescriptorHandle mDescriptorHandle;

		const std::vector<VkPushConstantRange>& mPushConstantRanges;

		uint8_t* mPushConstantBuffer;
	};

}
