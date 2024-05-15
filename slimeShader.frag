#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D trailTexture;
uniform sampler2D agentTexture;
uniform float time;

void main()
{
	vec4 agentTex = texture(agentTexture, TexCoords);
	vec4 trailTex = texture(trailTexture, TexCoords);

	FragColor = agentTex + trailTex;
};