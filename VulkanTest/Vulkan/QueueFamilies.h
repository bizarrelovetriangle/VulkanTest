#pragma once
#include <set>
#include <vulkan/vulkan.hpp>

class QueueFamilies {
public:
    QueueFamilies(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface);
    void findQueueFamilies();

	struct QueueFamily {
		std::set<vk::QueueFlagBits> flags;
		size_t count;
		uint32_t index;
		bool presentSupport;
	};

public:
    uint32_t graphicQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    uint32_t transferQueueFamilyIndex;

	vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue transferQueue;
    std::vector<QueueFamily> queueFamilies;

private:
    vk::PhysicalDevice& physicalDevice;
    vk::SurfaceKHR& surface;

    std::vector<vk::QueueFlagBits> queueFlags = {
        vk::QueueFlagBits::eGraphics,
        vk::QueueFlagBits::eCompute,
        vk::QueueFlagBits::eTransfer,
        vk::QueueFlagBits::eSparseBinding,
        vk::QueueFlagBits::eProtected,
        vk::QueueFlagBits::eVideoDecodeKHR,
        vk::QueueFlagBits::eOpticalFlowNV
    };
};