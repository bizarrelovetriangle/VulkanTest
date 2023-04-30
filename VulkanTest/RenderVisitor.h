#pragma once
class CommandBuffer;
class RenderObject;
class ColoredRenderObject;
class TexturedRenderObject;
template <class T>
class VertexedRenderObject;
class VulkanContext;
class Pipeline;
class VertexData;

class RenderVisitor
{
public:
	RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex);

	void Visit(const RenderObject& renderObject);
	template <class T>
	void Visit(const VertexedRenderObject<T>& renderObject);

private:
	void BindPipeline(Pipeline& pipeline);

	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
