#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"

#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

class RenderObjectVertexData
{
public:
	Vector3f position;
	Vector3f normal;
	Vector2f textureCoord;

	static vk::VertexInputBindingDescription BindingDescription()
	{
		return vk::VertexInputBindingDescription(0, sizeof(RenderObjectVertexData), vk::VertexInputRate::eVertex);
	}

	static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions()
	{
		vk::VertexInputAttributeDescription position(
			0, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, position));
		vk::VertexInputAttributeDescription normal(
			1, 0, vk::Format::eR32G32B32Sfloat, offsetof(RenderObjectVertexData, normal));
		vk::VertexInputAttributeDescription textureCoord(
			2, 0, vk::Format::eR32G32Sfloat, offsetof(RenderObjectVertexData, textureCoord));

		return { position, normal, textureCoord };
	}
};

class RenderObject
{
public:
	std::string name;
	std::vector<RenderObjectVertexData> vertexData;
};