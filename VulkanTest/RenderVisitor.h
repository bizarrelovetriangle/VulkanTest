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

	void Visit(Renderer& Renderer);
	void Visit(VertexedRenderer& Renderer);

private:
	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
