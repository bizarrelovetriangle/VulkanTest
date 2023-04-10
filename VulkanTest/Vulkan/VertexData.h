#pragma once;

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vector>

class VertexData
{
public:
    struct Vertex { float x, y; };
    struct Color { float r, g, b; };

    Vertex pos;
    Color color;

    static vk::VertexInputBindingDescription BindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions()
    {
        vk::VertexInputAttributeDescription position(0, 0, vk::Format::eR32G32Sfloat, offsetof(VertexData, pos));
        vk::VertexInputAttributeDescription color(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, color));
        return { position, color };
    }
};