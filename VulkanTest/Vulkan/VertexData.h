#pragma once;

#define VK_HEADER_VERSION 239
#define VULKAN_HPP_NO_CONSTRUCTORS
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
        vk::VertexInputBindingDescription bindingDescription
        {
            .binding = 0,
            .stride = sizeof(VertexData),
            .inputRate = vk::VertexInputRate::eVertex
        };
        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions()
    {
        vk::VertexInputAttributeDescription position
        {
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32Sfloat,
            .offset = offsetof(VertexData, pos)
        };

        vk::VertexInputAttributeDescription color
        {
            .location = 1,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(VertexData, color)
        };

        return { position, color };
    }
};