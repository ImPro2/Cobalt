#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace Cobalt
{

	class Shader
	{
	public:
		Shader(const std::string& spvFilePath, VkShaderStageFlags stage);
		Shader(const std::vector<char>& spirv, VkShaderStageFlags stage);
		~Shader();

	public:
		VkShaderModule GetShaderModule() const { return mShaderModule; }

	private:
		std::vector<char> ReadSpirv(const std::string& spvFilePath);

		VkShaderModule CreateShaderModule(VkShaderStageFlags stage);
		void DestroyShaderModule();

	private:
		std::vector<char> mSpirvBinary;

		VkShaderModule mShaderModule;
	};

}