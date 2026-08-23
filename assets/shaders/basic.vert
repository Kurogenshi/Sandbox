#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec3 ourColor;
layout (location = 1) out vec2 ourTexCoord;

void main()
{
	ourColor = aColor;
	ourTexCoord = aTexCoord;
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}