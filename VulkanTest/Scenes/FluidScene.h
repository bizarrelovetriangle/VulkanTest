#pragma once
#include "Scene.h"
#include "../Objects/FluidObject.h"

class VulkanContext;

class FluidScene : public Scene
{
public:
	FluidScene(VulkanContext& vulkanContext, std::vector<std::shared_ptr<Object>>& objects) : Scene(vulkanContext, objects)
	{
		auto fluid = std::make_unique<FluidObject>(vulkanContext);
		objects.push_back(std::move(fluid));
	}
};
