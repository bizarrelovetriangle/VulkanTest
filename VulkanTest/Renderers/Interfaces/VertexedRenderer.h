#pragma once
#include "Renderer.h"

struct DeserializedObjectVertexData;
class MeshModel;

class VertexData
{
public:
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector3f position;
	Vector3f normal;
};

class VertexedRenderer : public Renderer
{
public:
	using VertexDataType = VertexData;
	VertexedRenderer(VulkanContext& vulkanContext);
	~VertexedRenderer();
	virtual void UpdateVertexBuffer(const MeshModel& mesh);
	virtual void Accept(RenderVisitor& renderVisitor, const Camera& camera);
	virtual void Dispose() override;

	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/vertexed.frag";

public:
	std::unique_ptr<BufferData> vertexBuffer;
};

