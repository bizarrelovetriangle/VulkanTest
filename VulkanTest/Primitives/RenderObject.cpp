#include "RenderObject.h"
#include "../RenderVisitor.h"
#include "../Vulkan/Memory/ImageMemory.h"
#include "../Vulkan/DescriptorSets.h"

RenderObject::RenderObject()
{

}

void RenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}

void RenderObject::Dispose()
{
	vertexBuffer->Dispose();
	uniformBuffer->Dispose();
	if (textureBuffer) textureBuffer->Dispose();
	descriptorSets->Dispose();
}
