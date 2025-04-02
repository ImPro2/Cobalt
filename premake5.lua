configurations
{
	"Debug",
	"Release",
	"Dist"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"]        = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["GLM"]         = "%{wks.location}/Dependencies/GLM/"
IncludeDir["stb_image"]   = "%{wks.location}/Dependencies/stb_image/include"
IncludeDir["ImGui"]       = "%{wks.location}/Dependencies/ImGui"
IncludeDir["VulkanSDK"]   = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

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
group ""

include "CobaltApp"