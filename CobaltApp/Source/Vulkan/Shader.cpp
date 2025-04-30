#include "Shader.hpp"
#include "GraphicsContext.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>

#include <shaderc/shaderc.hpp>
#include <spirv-tools/linker.hpp>

namespace Cobalt
{

	class ShadercIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		ShadercIncluder() = default;
		~ShadercIncluder() = default;

		virtual shaderc_include_result* GetInclude(const char* requested_source,
                                               shaderc_include_type type,
                                               const char* requesting_source,
                                               size_t include_depth) override;

		virtual void ReleaseInclude(shaderc_include_result* data) override;

	private:
		static std::string ReadFile(const std::string& filePath);
	};

	shaderc_include_result* ShadercIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth)
	{
		shaderc_include_result* result = new shaderc_include_result;

		std::string name = (std::filesystem::path(requesting_source).parent_path() / std::filesystem::path(requested_source)).string();
		std::string content = ReadFile(name);

		auto* userData = new std::pair<std::string, std::string>({ name, content });

		result->source_name = userData->first.data();
		result->source_name_length = userData->first.size();
		result->content = userData->second.data();
		result->content_length = userData->second.size();
		result->user_data = userData;

		return result;
	}

	void ShadercIncluder::ReleaseInclude(shaderc_include_result* data)
	{
		delete static_cast<std::pair<std::string, std::string>*>(data->user_data);
		delete data;
	}

	std::string ShadercIncluder::ReadFile(const std::string& filePath)
	{
		std::ifstream stream(filePath, std::ios::binary);
		std::string src;

		if (stream)
		{
			stream.seekg(0, std::ios::end);
			size_t size = stream.tellg();

			src.resize(size);
			stream.seekg(0, std::ios::beg);
			stream.read(src.data(), size);

			return src;
		}

		return "";
	}

	static uint32_t SizeOfVkFormat(VkFormat type)
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

	Shader::Shader(const std::string& glslFilePath)
		: mFileName(glslFilePath)
	{
		auto stageSrcMap = ParseFile(glslFilePath);

		mSpirvBinaries.reserve(stageSrcMap.size());

		for (auto& [stage, src] : stageSrcMap)
		{
			mSpirvBinaries[stage] = CompileShader(stage, src);
		}

		CreateShaderModules();
		CreateReflectionShaderModules();

		ReflectVertexInputLayout();
		ReflectDescriptorSetLayouts();
		ReflectPushConstants();
	}

#if 0
	Shader::Shader(const std::string& spvFilePath, VkShaderStageFlags stage)
		: mShaderStageFlags(stage)
	{
		ReadSpirv(spvFilePath);
		CreateShaderModule();

		SpvReflectResult result = spvReflectCreateShaderModule(mSpirvBinary.size(), mSpirvBinary.data(), &mReflectShaderModule);

		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			std::cerr << "SPIRV Reflection Error";
			return;
		}

		ReflectVertexInputLayout();
		ReflectDescriptorSetLayouts();
		ReflectPushConstants();
	}
#endif

	Shader::~Shader()
	{
		DestroyShaderModules();

		for (VkDescriptorSetLayout descriptorSetLayout : mDescriptorSetLayouts)
			vkDestroyDescriptorSetLayout(GraphicsContext::Get().GetDevice(), descriptorSetLayout, nullptr);

		for (auto [stage, reflectModule] : mReflectShaderModules)
			spvReflectDestroyShaderModule(&reflectModule);
	}

	std::unordered_map<VkShaderStageFlags, std::string> Shader::ParseFile(const std::string& filePath)
	{
		std::unordered_map<VkShaderStageFlags, std::string> stageSrcMap;
		VkShaderStageFlags currentStage = (VkShaderStageFlags)0;

		std::ifstream stream(filePath);

		if (!stream.good())
		{
			std::cout << "Invalid file path";
		}

		std::string line;
		while (std::getline(stream, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
					currentStage = VK_SHADER_STAGE_VERTEX_BIT;
				if (line.find("fragment") != std::string::npos)
					currentStage = VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			else
			{
				stageSrcMap[currentStage] += (line + '\n');
			}
		}

		return stageSrcMap;
	}

	std::vector<uint32_t> Shader::CompileShader(VkShaderStageFlags stage, const std::string& source)
	{
		shaderc::Compiler compiler;

		shaderc_shader_kind shaderKind;

		switch (stage)
		{
			case VK_SHADER_STAGE_VERTEX_BIT: shaderKind = shaderc_vertex_shader; break;
			case VK_SHADER_STAGE_FRAGMENT_BIT: shaderKind = shaderc_fragment_shader; break;
		}

		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetIncluder(std::make_unique<ShadercIncluder>());
		options.SetTargetSpirv(shaderc_spirv_version_1_6);

		shaderc::PreprocessedSourceCompilationResult preprocessedResult = compiler.PreprocessGlsl(source, shaderKind, mFileName.c_str(), options);

		if (preprocessedResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << "Shader Preprocessor Error: " << preprocessedResult.GetErrorMessage() << std::endl;
		}

		std::string preprocessedSrc = std::string(preprocessedResult.begin(), preprocessedResult.end());

		shaderc::CompilationResult compileResult = compiler.CompileGlslToSpv(preprocessedSrc, shaderKind, mFileName.data(), options);

		if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << "Shader Compilation Error: " << compileResult.GetErrorMessage() << std::endl;
		}

		return std::vector<uint32_t>(compileResult.begin(), compileResult.end());
	}

#if 0
	std::vector<uint32_t> Shader::LinkSpirvs(const std::vector<std::vector<uint32_t>>& spirvs)
	{
		spvtools::Context context(SPV_ENV_VULKAN_1_3);
		spvtools::LinkerOptions linkerOptions;
		linkerOptions.SetAllowPtrTypeMismatch(true);
		linkerOptions.SetUseHighestVersion(true);

		context.SetMessageConsumer([](spv_message_level_t messageLevel, const char*, const spv_position_t& pos, const char* msg)
		{
			//if (messageLevel == SPV_MSG_ERROR || messageLevel == SPV_MSG_FATAL)
			//{
				std::cerr << "SPIRV Linking Error: " << msg << std::endl;
			//}
		});

		std::vector<uint32_t> linkedSpirv;
		spv_result_t result = spvtools::Link(context, spirvs, &linkedSpirv, linkerOptions);

		return linkedSpirv;
	}
#endif

	void Shader::ReflectVertexInputLayout()
	{
		if (mReflectShaderModules.find(VK_SHADER_STAGE_VERTEX_BIT) == mReflectShaderModules.end())
			return;

		uint32_t offset = 0;
		uint32_t stride = 0;

		for (uint32_t i = 0; i < mReflectShaderModules[VK_SHADER_STAGE_VERTEX_BIT].input_variable_count; i++)
		{
			const SpvReflectInterfaceVariable* inputVariable = mReflectShaderModules[VK_SHADER_STAGE_VERTEX_BIT].input_variables[i];

			if (std::string(inputVariable->name).find("gl_") != std::string::npos)
				continue;

			mVertexInputAttributeDescriptions.push_back({
				.location = inputVariable->location,
				.binding = 0,
				.format = (VkFormat)inputVariable->format,
				.offset = offset
			});

			uint32_t size = SizeOfVkFormat((VkFormat)inputVariable->format);

			offset += size;
			stride += size;
		}

		mVertexInputBindingDescription = {
			.binding = 0,
			.stride = stride,
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
	}

	void Shader::ReflectDescriptorSetLayouts()
	{
		std::vector<std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings;
		descriptorSetLayoutBindings.resize(mReflectShaderModules[VK_SHADER_STAGE_VERTEX_BIT].descriptor_set_count);

		for (const auto& [stage, reflectModule] : mReflectShaderModules)
		{
			for (uint32_t j = 0; j < reflectModule.descriptor_set_count; j++)
			{
				const SpvReflectDescriptorSet& descriptorSet = reflectModule.descriptor_sets[j];

				for (uint32_t k = 0; k < descriptorSet.binding_count; k++)
				{
					const SpvReflectDescriptorBinding* descriptorBinding = descriptorSet.bindings[k];

					VkShaderStageFlags stageFlags = (VkShaderStageFlags)reflectModule.shader_stage;

					// Add shader stage flag if the binding already exists

					if (descriptorBinding->set < descriptorSetLayoutBindings.size())
					{
						auto bindingsMap = descriptorSetLayoutBindings[descriptorBinding->set];

						if (bindingsMap.find(descriptorBinding->binding) != bindingsMap.end())
						{
							VkShaderStageFlags otherStageFlags = bindingsMap.at(descriptorBinding->binding).stageFlags;
							stageFlags |= otherStageFlags;
						}
					}

					VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
						.binding = descriptorBinding->binding,
						.descriptorType = (VkDescriptorType)descriptorBinding->descriptor_type,
						.descriptorCount = descriptorBinding->count,
						.stageFlags = stageFlags
					};

					if (descriptorSetLayoutBinding.descriptorCount == 0)
						descriptorSetLayoutBinding.descriptorCount = CO_BINDLESS_DESCRIPTOR_COUNT;

					descriptorSetLayoutBindings[descriptorBinding->set][descriptorBinding->binding] = descriptorSetLayoutBinding;
				}
			}
		}

		for (const auto& bindingsMap : descriptorSetLayoutBindings)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(bindingsMap.size());
			std::transform(bindingsMap.begin(), bindingsMap.end(), bindings.begin(), [](auto pair) { return pair.second; });

			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = (uint32_t)bindings.size(),
				.pBindings = bindings.data(),
			};

			VkDescriptorSetLayout descriptorSetLayout;

			VK_CALL(vkCreateDescriptorSetLayout(GraphicsContext::Get().GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

			mDescriptorSetLayouts.push_back(descriptorSetLayout);
		}
	}

	void Shader::ReflectPushConstants()
	{
		std::vector<VkPushConstantRange> pushConstantRanges;

		for (const auto& [stage, reflectModule] : mReflectShaderModules)
		{
			for (uint32_t i = 0; i < reflectModule.push_constant_block_count; i++)
			{
				const SpvReflectBlockVariable& pushConstantBlock = reflectModule.push_constant_blocks[i];

				VkPushConstantRange pushConstantRange = {
					.stageFlags = stage,
					.offset = pushConstantBlock.offset,
					.size = pushConstantBlock.size,
				};

				pushConstantRanges.push_back(pushConstantRange);
			}
		}

		if (pushConstantRanges.size() == 1)
		{
			mPushConstantRanges = pushConstantRanges;
			return;
		}

		for (uint32_t i = 1; i < pushConstantRanges.size(); i++)
		{
			if (pushConstantRanges[i].offset == pushConstantRanges[i - 1].offset)
			{
				VkPushConstantRange pushConstantRange = pushConstantRanges[i];
				pushConstantRange.stageFlags |= pushConstantRanges[i - 1].stageFlags;

				mPushConstantRanges.push_back(pushConstantRange);
			}
			else
			{
				mPushConstantRanges.push_back(pushConstantRanges[i]);
			}
		}
	}

	void Shader::CreateShaderModules()
	{
		for (auto [stage, spirv] : mSpirvBinaries)
		{
			VkShaderModuleCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = 4 * spirv.size(),
				.pCode = reinterpret_cast<const uint32_t*>(spirv.data())
			};

			VK_CALL(vkCreateShaderModule(GraphicsContext::Get().GetDevice(), &createInfo, nullptr, &mShaderModules[stage]));
		}
	}

	void Shader::CreateReflectionShaderModules()
	{
		for (auto [stage, spirv] : mSpirvBinaries)
		{
			SpvReflectResult result = spvReflectCreateShaderModule(4 * spirv.size(), spirv.data(), &mReflectShaderModules[stage]);

			if (result != SPV_REFLECT_RESULT_SUCCESS)
			{
				std::cerr << "SPIRV Reflection Error";
				return;
			}
		}

	}

	void Shader::DestroyShaderModules()
	{
		for (auto& [stage, module] : mShaderModules)
		{
			vkDestroyShaderModule(GraphicsContext::Get().GetDevice(), module, nullptr);
		}
	}

}