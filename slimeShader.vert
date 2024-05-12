#version 330 core

layout (location = 0) in vec3 agent;

void main()
{
	gl_Position = vec4(agent.x, agent.y, 0.0, 1.0);
};