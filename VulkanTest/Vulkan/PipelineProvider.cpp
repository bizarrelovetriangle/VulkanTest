#include "PipelineProvider.h"
#include "Pipeline.h"
#include "DescriptorSets.h"
#include "../VulkanContext.h"
#include "../Primitives/RenderObject.h";
#include "../Primitives/VertexedRenderObject.h"
#include "../Primitives/ColoredRenderObject.h"
#include "../Primitives/TexturedRenderObject.h"

PipelineProvider::PipelineProvider(VulkanContext& vulkanContext)
	: vulkanContext(vulkanContext)
{
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const RenderObject& renderObject)
{
	throw std::exception("gosh");
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const ColoredRenderObject& renderObject)
{
	std::type_index typeId = typeid(renderObject);

	if (auto it = pipelines.emplace(typeId, nullptr); it.second)
	{
		auto vertexDataBinding = ColoredRenderObject::VertexDataType::BindingDescription();
		auto vertexDataAttributes = ColoredRenderObject::VertexDataType::AttributeDescriptions();
		auto& descriptorLayout = renderObject.descriptorSets->descriptorSetLayout;
		it.first->second = std::make_shared<Pipeline>(
			vulkanContext, descriptorLayout, vertexDataBinding, vertexDataAttributes);
	}

	return pipelines.at(typeId);
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const TexturedRenderObject& renderObject)
{
	std::type_index typeId = typeid(renderObject);

	if (auto it = pipelines.emplace(typeId, nullptr); it.second)
	{
		auto vertexDataBinding = TexturedRenderObject::VertexDataType::BindingDescription();
		auto vertexDataAttributes = TexturedRenderObject::VertexDataType::AttributeDescriptions();
		auto& descriptorLayout = renderObject.descriptorSets->descriptorSetLayout;
		it.first->second = std::make_shared<Pipeline>(
			vulkanContext, descriptorLayout, vertexDataBinding, vertexDataAttributes);
	}

	return pipelines.at(typeId);
}

std::shared_ptr<Pipeline> PipelineProvider::GetPipeline(const VertexedRenderObject<VertexData>& renderObject)
{
	std::type_index typeId = typeid(renderObject);

	if (auto it = pipelines.emplace(typeId, nullptr); it.second)
	{
		auto vertexDataBinding = TexturedRenderObject::VertexDataType::BindingDescription();
		auto vertexDataAttributes = TexturedRenderObject::VertexDataType::AttributeDescriptions();
		auto& descriptorLayout = renderObject.descriptorSets->descriptorSetLayout;
		it.first->second = std::make_shared<Pipeline>(
			vulkanContext, descriptorLayout, vertexDataBinding, vertexDataAttributes);
	}

	return pipelines.at(typeId);
}

void PipelineProvider::Dispose()
{
	for (auto& pipeline : pipelines) pipeline.second->Dispose();
}
