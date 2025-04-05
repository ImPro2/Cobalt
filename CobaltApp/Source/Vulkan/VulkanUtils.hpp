#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <functional>

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

#ifdef CO_DEBUG
#define VK_CALL(fn) CheckVkResult(fn, __FILE__, __LINE__, __FUNCTION__)
#else
#define VK_CALL(fn)
#endif

namespace Cobalt
{

	inline void CheckVkResult(VkResult result, const char* file, int32_t line, const char* functionStr)
	{
		if (result != VK_SUCCESS)
		{
			std::cerr << "Vulkan Error (" << result << ") in " << file << ":" << line << ":" << functionStr << '\n';
			__debugbreak();
		}
	}

}


