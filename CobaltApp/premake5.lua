project "CobaltApp"
	kind "ConsoleApp"
	
	links
	{
        "ImGui",
        "GLFW",
        "stb_image",
		"assimp",
        "VkBootstrap",
		"spv_reflect",
		"%{Library.Vulkan}",
		"%{Library.shaderc}",
		"%{Library.SPIRVTools}",
		"%{Library.Optick}"
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
		"%{IncludeDir.assimp}",
        "%{IncludeDir.VkBootstrap}",
		"%{IncludeDir.spv_reflect}",
		"%{IncludeDir.Optick}"
	}

	postbuildcommands
	{
		"{COPY} %{LibraryDir.Optick}/OptickCore.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}"
	}