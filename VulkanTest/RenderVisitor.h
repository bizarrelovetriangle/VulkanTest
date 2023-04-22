class CommandBuffer;
class RenderObject;
class VulkanContext;
class Pipeline;

class RenderVisitor
{
public:
	RenderVisitor(VulkanContext& vulkanContext, CommandBuffer& commandBuffer, size_t imageIndex);

	void Visit(const RenderObject& renderObject);

private:
	void BindPipeline(Pipeline& pipeline);

	VulkanContext& vulkanContext;
	vk::CommandBuffer& commandBuffer;
	size_t imageIndex;
};
