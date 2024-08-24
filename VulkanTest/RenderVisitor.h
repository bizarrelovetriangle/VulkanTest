#pragma once
namespace vk
{
	class CommandBuffer;
}
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
	RenderVisitor(VulkanContext& vulkanContext, vk::CommandBuffer& commandBuffer, size_t imageIndex);

	void Visit(Renderer& renderer);
	void Visit(VertexedRenderer& renderer);

//private:
	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
