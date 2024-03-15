#pragma once
class CommandBuffer;
class RenderObject;
class ColoredRenderObject;
class TexturedRenderObject;
class VertexedRenderObject;
class VulkanContext;
class Pipeline;
class VertexData;
class PlaneRenderObject;
class Camera;

class RenderVisitor
{
public:
	RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex);

	void Visit(RenderObject& renderObject, const Camera& camera);
	void Visit(PlaneRenderObject& renderObject, const Camera& camera);
	void Visit(VertexedRenderObject& renderObject, const Camera& camera);

private:
	void BindPipeline(Pipeline& pipeline);

	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
