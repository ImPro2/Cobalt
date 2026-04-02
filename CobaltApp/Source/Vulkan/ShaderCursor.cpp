#include "copch.hpp"
#include "ShaderCursor.hpp"
#include "DescriptorCache.hpp"
#include "GraphicsContext.hpp"

namespace Cobalt
{

	ShaderCursor::ShaderCursor(ShaderParameter& shaderParameter, DescriptorHandle descriptorHandle, const std::vector<VkPushConstantRange>& pushConstantRanges, uint8_t* pushConstantBuffer)
		: mShaderParameter(shaderParameter), mDescriptorBindings{}, mDescriptorBindingsRef(mDescriptorBindings), mDescriptorHandle(descriptorHandle),
		  mPushConstantRanges(pushConstantRanges), mPushConstantBuffer(pushConstantBuffer)
	{
		CO_PROFILE_FN();
	}

	ShaderCursor::ShaderCursor(const ShaderCursor& other)
		: mShaderParameter(other.mShaderParameter), mDescriptorBindingsRef(other.mDescriptorBindingsRef), mDescriptorHandle(other.mDescriptorHandle),
		  mPushConstantRanges(other.mPushConstantRanges), mPushConstantBuffer(other.mPushConstantBuffer)
	{
	}

	ShaderCursor::ShaderCursor(ShaderParameter& shaderParameter, DescriptorBindings& descriptorBindings, DescriptorHandle descriptorHandle, const std::vector<VkPushConstantRange>& pushConstantRanges, uint8_t* pushConstantBuffer)
		: mShaderParameter(shaderParameter), mDescriptorBindingsRef(descriptorBindings), mDescriptorHandle(descriptorHandle),
		  mPushConstantRanges(pushConstantRanges), mPushConstantBuffer(pushConstantBuffer)
	{
		CO_PROFILE_FN();
	}

	ShaderCursor::~ShaderCursor()
	{
		CO_PROFILE_FN();
	}
	
	void ShaderCursor::Write(const void* data, size_t size)
	{
		CO_PROFILE_FN();

		if (mShaderParameter.IsPushConstant)
		{
			assert(size == mShaderParameter.UniformSize);

			size_t pushConstantRangeOffset = mPushConstantRanges[mShaderParameter.PushConstantRangeIndex].offset + mShaderParameter.UniformByteOffset;
			void* dst = mPushConstantBuffer + pushConstantRangeOffset;
			memcpy(dst, data, size);
		}
		else
		{
			// TODO
		}
	}

	void ShaderCursor::Write(const VulkanBuffer& buffer)
	{
		CO_PROFILE_FN();

		assert(mShaderParameter.Kind == ShaderParameterKind::UniformBuffer || mShaderParameter.Kind == ShaderParameterKind::StorageBuffer);

		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;

		if (buffer.GetUsageFlags() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		{
			descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (buffer.GetUsageFlags() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		{
			descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}

		mDescriptorBindingsRef.Bindings.push_back(DescriptorBinding {
			.Binding = mShaderParameter.Binding,
			.Element = mShaderParameter.Index,
			.DescriptorType = descriptorType,
			.Address = buffer.GetDeviceAddress(),
			.Range = buffer.GetAllocationInfo().size
		});
	}

	void ShaderCursor::Write(const Texture& texture)
	{
		CO_PROFILE_FN();

		Write(Image {
			texture.GetSampler(), texture.GetImageView(), texture.GetImageLayout()
		});
	}

	void ShaderCursor::Write(const Cubemap& cubemap)
	{
		CO_PROFILE_FN();

		Write(Image {
			cubemap.GetSampler(), cubemap.GetImageView(), cubemap.GetImageLayout()
		});
	}

	void ShaderCursor::Write(const Image& image)
	{
		CO_PROFILE_FN();

		//assert(mShaderParameter.Kind == ShaderParameterKind::CombinedImageSampler);

		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		mDescriptorBindingsRef.Bindings.push_back(DescriptorBinding {
			.Binding = mShaderParameter.Binding,
			.Element = mShaderParameter.Index,
			.DescriptorType = descriptorType,
			.Sampler = image.Sampler,
			.ImageView = image.ImageView,
			.ImageLayout = image.ImageLayout 
		});
	}

	void ShaderCursor::Write(const std::vector<Image>& images)
	{
		CO_PROFILE_FN();
		assert(mShaderParameter.Kind == ShaderParameterKind::Array);

		for (uint32_t i = 0; i < images.size(); i++)
		{
			Element(i).Write(images[i]);
		}
	}

	void ShaderCursor::Finalize()
	{
		CO_PROFILE_FN();

		auto& descriptorCache = GraphicsContext::Get().GetDescriptorCache();
		descriptorCache.WriteDescriptorBindingsIfNeeded(mDescriptorHandle, mDescriptorBindingsRef);

		//free(mPushConstantBuffer);
	}

}