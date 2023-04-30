#pragma once
#include <typeindex>
#include <unordered_map>
#include <memory>

class VulkanContext;
class RenderObject;
class ColoredRenderObject;
class TexturedRenderObject;
template <class T>
class VertexedRenderObject;
class Pipeline;
class VertexData;

class PipelineProvider
{
public:
	PipelineProvider(VulkanContext& vulkanContext);
	std::shared_ptr<Pipeline> GetPipeline(const RenderObject& renderObject);
	void Dispose();

private:
	template <class T>
	void CreatePipeline();

	VulkanContext& vulkanContext;
	std::unordered_map<std::type_index, std::shared_ptr<Pipeline>> pipelines;
};

