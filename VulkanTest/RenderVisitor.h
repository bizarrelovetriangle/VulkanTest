class CommandBuffer;
class RenderObject;
class Pipeline;

class RenderVisitor
{
public:
	RenderVisitor(CommandBuffer& commandBuffer, Pipeline& pipeline);

	void Visit(const RenderObject& renderObject);

private:
	void BindPipeline(Pipeline& pipeline);

	vk::CommandBuffer& commandBuffer;
	Pipeline& pipeline;
};
