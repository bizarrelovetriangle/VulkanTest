#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

class QueueFamilies {
public:
    QueueFamilies(vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface);
    void findQueueFamilies();

	struct QueueFamily {
		std::unordered_set<vk::QueueFlagBits> flags;
		size_t count;
		uint32_t index;
		bool presentSupport;
	};

public:
    uint32_t graphicQueueFamily;
    uint32_t presentQueueFamily;
    uint32_t transferQueueFamily;

    std::unordered_map<uint32_t, vk::Queue> queueMap;
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