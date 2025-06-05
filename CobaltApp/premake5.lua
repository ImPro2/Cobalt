project "CobaltApp"
	kind "ConsoleApp"
	
	links
	{
        "ImGui",
        "GLFW",
        "stb_image",
		"assimp",
		"%{Library.slang}",
		"%{Library.Vulkan}",
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
		"%{IncludeDir.Optick}"
	}

	postbuildcommands
	{
		"{COPY} %{LibraryDir.Optick}/OptickCore.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}"
	}