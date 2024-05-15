#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;

void main()
{
	//TexCoords = (aPos.xy / 2) + 0.5;
	TexCoords = texCoords;
	gl_Position = vec4(aPos, 1.0);
};