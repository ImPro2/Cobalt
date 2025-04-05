#include "VulkanUtils.hpp"

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
	std::cerr << "Vulkan Message (" << messageSeverity << ":" << messageType << "): " << callbackData->pMessage << '\n\n';

	return VK_FALSE;
}
