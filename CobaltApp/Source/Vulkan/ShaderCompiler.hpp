#pragma once
#include "Shader.hpp"

#include <iostream>
#include <vector>
#include <string>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>

namespace Cobalt
{

	class ShaderCompiler
	{
	public:
		static void Init();
		static void Shutdown();

	public:
		// returns linked program
		static Slang::ComPtr<slang::IComponentType> CompileShader(const std::string& filePath);

	private:
		inline static Slang::ComPtr<slang::IGlobalSession> sGlobalSession;
		inline static Slang::ComPtr<slang::ISession> sDefaultSession;
	};

}
