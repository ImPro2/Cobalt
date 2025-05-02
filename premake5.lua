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
IncludeDir["assimp"]        = "%{wks.location}/Dependencies/assimp/include"
IncludeDir["VulkanSDK"]     = "%{VULKAN_SDK}/Include"
IncludeDir["VkBootstrap"]   = "%{wks.location}/Dependencies/VkBootstrap/src"
IncludeDir["spv_reflect"]   = "%{wks.location}/Dependencies/spv_reflect"
IncludeDir["Optick"]        = "%{wks.location}/Dependencies/Optick/src"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["Optick"] = "%{wks.location}/Dependencies/Optick/bin/vs2022/x64/Release"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["shaderc"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRVTools"] = "%{LibraryDir.VulkanSDK}/SPIRV-Tools-link.lib"
Library["Optick"] = "%{LibraryDir.Optick}/OptickCore.lib"

workspace "Cobalt"
	architecture "x64"
	startproject "CobaltApp"
	language "C++"
	cppdialect "C++latest"
	staticruntime "Off"
	runtime "Release"

	flags { "MultiProcessorCompile" }

	targetdir ("%{wks.location}/bin/"     .. outputdir .. "/%{prj.name}")
	objdir    ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	debugdir "%{wks.location}"

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "CO_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "CO_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "CO_DIST"
		optimize "On"

    filter { "system:linux" }
        buildoptions { "-Wno-return-type" }

group "Dependencies"
    include "Dependencies/GLFW"
    include "Dependencies/ImGui"
    include "Dependencies/stb_image"
	include "Dependencies/VkBootstrap"
	include "Dependencies/assimp"
	include "Dependencies/spv_reflect"
group ""

include "CobaltApp"