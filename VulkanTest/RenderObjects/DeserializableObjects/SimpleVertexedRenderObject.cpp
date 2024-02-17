#include "SimpleVertexedRenderObject.h"
#include "../../RenderVisitor.h"
#include "../../Vulkan/DescriptorSets.h"
#include "../../Utils/GLTFReader.h"
#include "../../Vulkan/Data/ImageData.h"
#include "../../Vulkan/Data/BufferData.h"
#include "../../RenderVisitor.h"
#include "../../VulkanContext.h"

SimpleVertexedRenderObject::SimpleVertexedRenderObject(VulkanContext& vulkanContext)
	: VertexedRenderObject(vulkanContext)
{
	shared = Shared<SimpleVertexedRenderObject>::getInstance(vulkanContext);
	descriptorSets = std::make_unique<DescriptorSets>(vulkanContext, shared->descriptorSetLayout);
	descriptorSets->UpdateUniformDescriptor(*transformUniformBuffer, 0);
	descriptorSets->UpdateUniformDescriptor(*propertiesUniformBuffer, 1);
}


