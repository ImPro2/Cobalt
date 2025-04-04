project "VkBootstrap"
	kind "StaticLib"

	targetdir ("%{wks.location}/bin/"     .. outputdir .. "/%{prj.name}")
	objdir    ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/stb_image.h",
		"src/VkBootstrap.cpp",
        "src/VkBootstrapDispatch.h"
	}

	includedirs
	{
		"src",
        "%{IncludeDir.VulkanSDK}"
	}

    links
    {
        "%{Library.Vulkan}"
    }