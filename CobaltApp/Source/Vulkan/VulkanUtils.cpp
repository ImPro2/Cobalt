#include "copch.hpp"
#include "VulkanUtils.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
	CO_PROFILE_FN();

	std::cout << "Vulkan Message (" << messageSeverity << ":" << messageType << "): " << callbackData->pMessage << '\n\n' << std::endl << std::endl;

	return VK_FALSE;
}
