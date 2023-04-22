#pragma once
#include <typeindex>
#include <unordered_map>
#include <memory>

class VulkanContext;
class RenderObject;
class Pipeline;

class PipelineProvider
{
public:
	PipelineProvider(VulkanContext& vulkanContext);
	std::shared_ptr<Pipeline> GetPipeline(const RenderObject& renderObject);
	void Dispose();

private:
	VulkanContext& vulkanContext;
	std::unordered_map<std::type_index, std::shared_ptr<Pipeline>> pipelines;
};

