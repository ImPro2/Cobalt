#pragma once
#include "OptickMacros.hpp"

#include <vulkan/vulkan.h>
#include <iostream>
#include <functional>

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

#define VK_CALL(fn) CheckVkResult(fn, __FILE__, __LINE__, __FUNCTION__)

namespace Cobalt
{

	inline void CheckVkResult(VkResult result, const char* file, int32_t line, const char* functionStr)
	{
		CO_PROFILE_FN();

		if (result != VK_SUCCESS)
		{
			std::cerr << "Vulkan Error (" << result << ") in " << file << ":" << line << ":" << functionStr << '\n';
			__debugbreak();
		}
	}

}


