#version 460 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec3 ourColor;
layout (location = 1) in vec2 ourTexCoord;

layout (binding = 0) uniform sampler2D ourTexture1;
layout (binding = 1) uniform sampler2D ourTexture2;

void main()
{
	FragColor = mix(texture(ourTexture1, ourTexCoord), texture(ourTexture2, ourTexCoord), 0.2);
}