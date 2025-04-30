#pragma once
#include <vulkan/vulkan.h>
#include "spirv_reflect.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace Cobalt
{

	class Shader
	{
	public:
		Shader(const std::string& glslFilePath);
		Shader(const std::string& spvFilePath, VkShaderStageFlags stage);
		~Shader();

	public:
		bool HasStage(VkShaderStageFlagBits stage) const
		{
			return mShaderStageFlags & stage;
		}

		std::unordered_map<VkShaderStageFlags, VkShaderModule> GetShaderModules() const { return mShaderModules; }

		const VkVertexInputBindingDescription& GetVertexInputBindingDescription() const { return mVertexInputBindingDescription; }

		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const { return mDescriptorSetLayouts; }
		const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributeDescriptions() const { return mVertexInputAttributeDescriptions; }

		const std::vector<VkPushConstantRange>& GetPushConstantRanges() const { return mPushConstantRanges; }

	private:
		std::unordered_map<VkShaderStageFlags, std::string> ParseFile(const std::string& filePath);
		std::vector<uint32_t> CompileShader(VkShaderStageFlags stage, const std::string& source);
		//std::vector<uint32_t> LinkSpirvs(const std::vector<std::vector<uint32_t>>& spirvs);

		void CreateShaderModules();
		void CreateReflectionShaderModules();

		//void ReadSpirv(const std::string& spvFilePath);
		void ReflectVertexInputLayout();
		void ReflectDescriptorSetLayouts();
		void ReflectPushConstants();


	public:
		void DestroyShaderModules();

	private:
		std::string mFileName;

		VkShaderStageFlags mShaderStageFlags;
		std::unordered_map<VkShaderStageFlags, std::vector<uint32_t>> mSpirvBinaries;

		std::unordered_map<VkShaderStageFlags, VkShaderModule> mShaderModules;
		std::unordered_map<VkShaderStageFlags, SpvReflectShaderModule> mReflectShaderModules;

		VkVertexInputBindingDescription mVertexInputBindingDescription;
		std::vector<VkVertexInputAttributeDescription> mVertexInputAttributeDescriptions;
		std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;

		std::vector<VkPushConstantRange> mPushConstantRanges;
	};

}