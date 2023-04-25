#include "PipelineProvider.h"
#include "Pipeline.h"
#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "../Primitives/RenderObject.h";
#include "../Primitives/VertexedRenderObject.h"
#include "../Primitives/ColoredRenderObject.h"
#include "../Primitives/TexturedRenderObject.h"
#include "../Utils/SingletonManager.h"
#include "DeviceController.h"
#include <vulkan/vulkan.hpp>

PipelineProvider::PipelineProvider(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
	CreatePipeline<VertexedRenderObject<VertexData>>();
	CreatePipeline<ColoredRenderObject>();
	CreatePipeline<TexturedRenderObject>();
}

template <class T>
void PipelineProvider::CreatePipeline()
{
	std::type_index typeId = typeid(T);

	if (auto it = pipelines.emplace(typeId, nullptr); it.second)
	{
		auto& shared = vulkanContext.singletonManager->Create<Shared<T>>(
			[&]() { return std::make_unique<Shared<T>>(vulkanContext); },
			[&](auto& shared) { shared.Dispose(); });

		it.first->second = std::make_shared<Pipeline>(vulkanContext, shared);
	}
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const RenderObject& renderObject)
{
	throw std::exception("gosh");
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const ColoredRenderObject& renderObject)
{
	std::type_index typeId = typeid(renderObject);
	return pipelines.at(typeId);
}


std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const TexturedRenderObject& renderObject)
{
	std::type_index typeId = typeid(renderObject);
	return pipelines.at(typeId);
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const VertexedRenderObject<VertexData>& renderObject)
{
	std::type_index typeId = typeid(renderObject);
	return pipelines.at(typeId);
}

void PipelineProvider::Dispose()
{
	for (auto& pipeline : pipelines) pipeline.second->Dispose();
}
