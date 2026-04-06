project "Cobalt"
	kind "StaticLib"
	flags { "MultiProcessorCompile" }

	files
	{
		"Source/**.cpp",
		"Source/**.hpp",
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.fastgltf}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.slang}"
	}

	links
	{
        "ImGui",
        "GLFW",
        "stb_image",
		"fastgltf",
		"%{Library.slang}",
		"%{Library.Vulkan}",
		"%{Library.Optick}"
	}