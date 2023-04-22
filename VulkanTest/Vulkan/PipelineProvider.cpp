#include "PipelineProvider.h"
#include "Pipeline.h"
#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "../Primitives/RenderObject.h";

PipelineProvider::PipelineProvider(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const RenderObject& renderObject)
{
	std::type_index typeId = typeid(renderObject);

	if (auto it = pipelines.emplace(typeId, nullptr); it.second)
	{
		auto& descriptorLayout = renderObject.descriptorSets->descriptorSetLayout;
		it.first->second = std::make_shared<Pipeline>(vulkanContext, descriptorLayout);
	}

	return pipelines.at(typeId);
}

void PipelineProvider::Dispose()
{
	for (auto& pipeline : pipelines) pipeline.second->Dispose();
}
