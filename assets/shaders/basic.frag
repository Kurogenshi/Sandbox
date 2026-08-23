#version 460 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 ourColor;

void main()
{
	FragColor = vec4(ourColor, 1.0);
}