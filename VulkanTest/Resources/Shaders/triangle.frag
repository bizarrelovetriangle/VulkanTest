#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexturePos;
layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 textureColor = texture(texSampler, fragTexturePos);
    outColor = vec4(fragTexturePos, 0., 1.0);
    outColor = textureColor;
}