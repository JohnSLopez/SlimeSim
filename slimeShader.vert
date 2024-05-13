#version 330 core

struct Agent
{
	vec2 pos;
	float angle;
};

layout (location = 0) in Agent agent;
uniform float time;

void main()
{
	vec2 newPosition = vec2(agent.pos.x + sin(time) * 0.5, agent.pos.y + sin(time) * 0.5);
	gl_Position = vec4(newPosition.x, newPosition.y, 0.0, 1.0);
};