project "CobaltApp"
	kind "ConsoleApp"
	
	links
	{
		"Cobalt",
        "ImGui",
        "GLFW",
        "stb_image",
		"fastgltf",
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
		"%{wks.location}/Cobalt/Source",
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

	postbuildcommands
	{
		"{COPY} %{LibraryDir.Optick}/OptickCore.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}"
	}