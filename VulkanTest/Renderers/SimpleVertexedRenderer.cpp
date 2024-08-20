#include "SimpleVertexedRenderer.h"
#include "../RenderVisitor.h"
#include "../Vulkan/DescriptorSets.h"
#include "../Utils/GLTFReader.h"
#include "../Vulkan/Data/ImageData.h"
#include "../Vulkan/Data/BufferData.h"
#include "../RenderVisitor.h"
#include "../VulkanContext.h"

SimpleVertexedRenderer::SimpleVertexedRenderer(VulkanContext& vulkanContext)
	: VertexedRenderer(vulkanContext)
{
	shared = Shared<SimpleVertexedRenderer>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout, DescriptorSetLayoutBinding());
	descriptorSets->UpdateUniformDescriptor(*vulkanContext.commonUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 1);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 2);
}


