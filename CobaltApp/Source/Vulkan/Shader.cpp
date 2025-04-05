#include "Shader.hpp"
#include "GraphicsContext.hpp"
#include <fstream>

namespace Cobalt
{

	Shader::Shader(const std::string& spvFilePath, VkShaderStageFlags stage)
		: Shader(ReadSpirv(spvFilePath), stage)
	{
	}

	Shader::Shader(const std::vector<char>& spirv, VkShaderStageFlags stage)
		: mSpirvBinary(spirv)
	{
		mShaderModule = CreateShaderModule(stage);
	}

	Shader::~Shader()
	{
		DestroyShaderModule();
	}

	std::vector<char> Shader::ReadSpirv(const std::string& spvFilePath)
	{
		std::ifstream stream(spvFilePath, std::ios::binary);

		stream.seekg(0, std::ios_base::end);
		std::streampos size = stream.tellg();
		stream.seekg(0, std::ios_base::beg);

		std::vector<char> buffer(size);
		stream.read(buffer.data(), size);
		stream.close();

		return buffer;
	}

	VkShaderModule Shader::CreateShaderModule(VkShaderStageFlags stage)
	{
		VkShaderModuleCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = mSpirvBinary.size(),
			.pCode = reinterpret_cast<const uint32_t*>(mSpirvBinary.data())
		};

		VkShaderModule shaderModule = VK_NULL_HANDLE;

		VK_CALL(vkCreateShaderModule(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	void Shader::DestroyShaderModule()
	{
		if (mShaderModule)
			vkDestroyShaderModule(GraphicsContext::Get().GetDevice(), mShaderModule, nullptr);
	}

}