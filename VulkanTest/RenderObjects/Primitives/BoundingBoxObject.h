#pragma once
#include "../Interfaces/VertexedRenderObject.h"
#include "../../Math/Vector3.h"
#include "../../Math/Plane.h"

class RenderVisitor;
class BufferData;

class LineVertexData
{
public:
	LineVertexData(const Vector3f& position);
	static vk::VertexInputBindingDescription BindingDescription();
	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

public:
	Vector3f position;
};

class BoundingBoxObject : public VertexedRenderObject
{
public:
	using VertexDataType = LineVertexData;
	// aa - front bottom left
	// bb - back top right
	BoundingBoxObject(VulkanContext& vulkanContext, const Vector3f& aa, const Vector3f& bb);
	~BoundingBoxObject();

public:
	inline static std::string VertexShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.vert";
	inline static std::string FragmentShader =
		"E:/Projects/VulkanTest/VulkanTest/Resources/Shaders/lined.frag";

private:
	std::array<Vector3f, 8> boundingPoints;
	std::array<Vector3f, 8> expandedBoundingPoints;
	std::vector<LineVertexData> vertexData;
};
