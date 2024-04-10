#pragma once
class CommandBuffer;
class Renderer;
class ColoredRenderer;
class TexturedRenderer;
class VertexedRenderer;
class VulkanContext;
class Pipeline;
class VertexData;
class PlaneRenderer;
class Camera;

class RenderVisitor
{
public:
	RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex);

	void Visit(Renderer& Renderer, const Camera& camera);
	void Visit(PlaneRenderer& Renderer, const Camera& camera);
	void Visit(VertexedRenderer& Renderer, const Camera& camera);

private:
	void BindPipeline(Pipeline& pipeline);

	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
