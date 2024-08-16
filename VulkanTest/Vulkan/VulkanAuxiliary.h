#pragma once
#include <vulkan/vulkan.hpp>
#include "ValidationLayersInfo.h"

class VulkanAuxiliary {
public:
	VulkanAuxiliary(ValidationLayersInfo& validationLayersInfo);

	void Dispose();

private:
	void createInstance();
	std::vector<const char*> getRequiredExtensions();

public:
	vk::DebugUtilsMessengerEXT debugMessenger;
	vk::Instance instance;
	vk::SurfaceKHR surface;

private:
	ValidationLayersInfo validationLayersInfo;

	struct DebugDispatch {
		int getVkHeaderVersion() const { return 268; };
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 0;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = 0;
	} debugDispatch;
};