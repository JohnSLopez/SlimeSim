#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include "VertexBuffer.h"
#include <iostream>
#include "Shader.h"
#include "ComputeShader.h"

unsigned int windowWidth = 1920;
unsigned int windowHeight = 1080;
const unsigned int numAgents = 1000000;

float currentTime = 0;
float prevTime = 0;
float deltaTime= 0;
# define PI 3.14159265358979323846  /* pi */

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

float mainTextureVertices[] =
{
		//position			//color			   //texture coords
		1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,		// top right
		1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,		// bot right
	   -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,		// bot left
	   -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f
};

unsigned int indices[] = { // order for drawing vertices
		0, 1, 3, // first trinagle
		1, 2, 3  // second triangle
};

struct Agent
{
	float x = 0.0f;
	float y = 0.0f;
	float direction = 0.0f;
};

int main()
{
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//create window object
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Slime Simulation", /*glfwGetPrimaryMonitor()*/ NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, windowWidth, windowHeight);

	//Generate agents with random direction and spawn in center of screen
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 2 * PI);

	Agent *agents;
	agents = (Agent*)malloc(numAgents * sizeof(Agent));

	for (int i = 0; i < numAgents; i++)
	{
		Agent temp;

		temp.x = windowWidth / 2;
		temp.y = windowHeight / 2;
		temp.direction = distribution(generator);

		/*int radius = windowHeight / 3;
		std::uniform_real_distribution<> randomAngle(0, 2 * PI);
		std::uniform_int_distribution<> randomRadius(0, radius);

		int distance = randomRadius(generator);
		float genAngle = randomAngle(generator);

		temp.x = (windowWidth / 2) + (cos(genAngle) * distance);
		temp.y = (windowHeight / 2) + (sin(genAngle) * distance);*/

		// get angle that is towards the circle centre
		//temp.direction = genAngle + PI;

		agents[i] = temp;
	}

	//Initialize Vertex Array Object
	unsigned int VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &EBO);

	//Initialize Vertex Buffers
	VertexBuffer textureTarget(mainTextureVertices, sizeof(mainTextureVertices));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attrib
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color attrib
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attrib
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	unsigned int ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	free(agents);

	//Create shaders
	Shader vfSlimeShader("slimeShader.vert", "slimeShader.frag");
	ComputeShader cSlimeShader("slimeShader.comp");

	//Create Textures
	const unsigned int TEXTURE_WIDTH = windowWidth, TEXTURE_HEIGHT = windowHeight;
	unsigned int textureArray[2];
	unsigned int trailTexture, agentTexture;

	//trail texture
	glGenTextures(1, &trailTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, trailTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//agent texture
	glGenTextures(1, &agentTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, agentTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	textureArray[0] = trailTexture;
	textureArray[1] = agentTexture;

	cSlimeShader.use();
	cSlimeShader.setUInt("windowWidth", windowWidth);
	cSlimeShader.setUInt("windowHeight", windowHeight);

	vfSlimeShader.use();
	vfSlimeShader.setInt("trailTexture", 0);
	vfSlimeShader.setInt("agentTexture", 1);
	vfSlimeShader.setInt("windowWidth", windowWidth);
	vfSlimeShader.setInt("windowHeight", windowHeight);

	glPointSize(10);
	glBindVertexArray(0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindTextures(0, 2, textureArray);

	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		//get deltaTime
		currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		//input
		processInput(window);
		
		//clear previous screen
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//compute shader
		cSlimeShader.use();
		cSlimeShader.setFloat("time", currentTime);
		glBindTextures(0, 2, textureArray);
		glBindImageTexture(3, trailTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(4, agentTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glDispatchCompute(numAgents, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		//vertex and frag shaders
		vfSlimeShader.use();
		vfSlimeShader.setFloat("time", currentTime);
		vfSlimeShader.setInt("trailTexture", 3);
		vfSlimeShader.setInt("agentTexture", 4);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}