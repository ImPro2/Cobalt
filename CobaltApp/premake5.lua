project "CobaltApp"
	kind "ConsoleApp"

	flags { "MultiProcessorCompile" }
	
	links
	{
        "ImGui",
        "GLFW",
        "stb_image",
        "VkBootstrap",
		"spv_reflect",
		"%{Library.Vulkan}",
		"%{Library.shaderc}",
		"%{Library.SPIRVTools}"
	}

	files
	{
		"Source/**.hpp",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.ImGui}",
        "%{IncludeDir.VkBootstrap}",
		"%{IncludeDir.spv_reflect}"
	}