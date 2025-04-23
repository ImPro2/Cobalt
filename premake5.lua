configurations
{
	"Debug",
	"Release",
	"Dist"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"]          = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["GLM"]           = "%{wks.location}/Dependencies/GLM/"
IncludeDir["stb_image"]     = "%{wks.location}/Dependencies/stb_image/include"
IncludeDir["ImGui"]         = "%{wks.location}/Dependencies/ImGui"
IncludeDir["Assimp"]        = "%{wks.location}/Dependencies/Assimp/include"
IncludeDir["VulkanSDK"]     = "%{VULKAN_SDK}/Include"
IncludeDir["VkBootstrap"]   = "%{wks.location}/Dependencies/VkBootstrap/src"
IncludeDir["spv_reflect"]   = "%{wks.location}/Dependencies/spv_reflect"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["shaderc"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRVTools"] = "%{LibraryDir.VulkanSDK}/SPIRV-Tools-link.lib"

workspace "Cobalt"
	architecture "x64"
	startproject "CobaltApp"
	language "C++"
	cppdialect "C++latest"
	staticruntime "Off"

	targetdir ("%{wks.location}/bin/"     .. outputdir .. "/%{prj.name}")
	objdir    ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		debugdir "%{wks.location}"
		defines "CO_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "CO_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "CO_DIST"
		runtime "Release"
		optimize "On"

    filter { "system:linux" }
        buildoptions { "-Wno-return-type" }

group "Dependencies"
    include "Dependencies/GLFW"
    include "Dependencies/ImGui"
    include "Dependencies/stb_image"
	include "Dependencies/VkBootstrap"
	include "Dependencies/Assimp"
	include "Dependencies/spv_reflect"
group ""

include "CobaltApp"