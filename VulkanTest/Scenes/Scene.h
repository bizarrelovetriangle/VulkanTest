#pragma once
#include "../VulkanContext.h"

class Scene
{
public:
	Scene(VulkanContext& vulkanContext, std::vector<std::shared_ptr<Object>>& objects) : objects(objects), vulkanContext(vulkanContext)
	{
	}

protected:
	std::vector<std::shared_ptr<Object>>& objects;
	VulkanContext& vulkanContext;
};