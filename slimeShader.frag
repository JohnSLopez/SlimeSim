#version 430 core

void diffuseTrail();

out vec4 FragColor;
in vec2 TexCoords;

layout (binding = 3, rgba32f) uniform image2D trailTexture;
layout (binding = 4, rgba32f) uniform image2D agentTexture;

uniform int windowWidth;
uniform int windowHeight;
uniform float time;
float diffuseRate = 0.3;
float decayRate = 0.0125;

void main()
{
	diffuseTrail();

	vec4 agentTex = imageLoad(agentTexture, ivec2(gl_FragCoord.xy)).rgba;
	vec4 trailTex = imageLoad(trailTexture, ivec2(gl_FragCoord.xy)).rgba;
	FragColor = trailTex;
};

void diffuseTrail()
{
	vec4 blurredValue;
	vec4 sum;
	vec4 originalColor = imageLoad(trailTexture, ivec2(gl_FragCoord.xy)).rgba;

	//loop through y axis
	for (int x = -1; x <= 1; x++)
	{
		//loop through x axis
		for (int y = -1; y <= 1; y++)
		{
			int sampleX = min(windowWidth-1, max(0, int(gl_FragCoord.x)+x));
			int sampleY = min(windowHeight-1, max(0, int(gl_FragCoord.y)+y));

			sum += imageLoad(trailTexture, ivec2(sampleX, sampleY)).rgba;
		}
	}
	blurredValue = sum / 9;

	float diffuseWeight = clamp(diffuseRate, 0, 1);
	vec4 calculatedColor = originalColor * (1 - diffuseWeight) + blurredValue * diffuseWeight;
	calculatedColor -= decayRate;
	calculatedColor.a = 1.0;
	imageStore(trailTexture, ivec2(gl_FragCoord.xy), max(calculatedColor, 0.0f));
};