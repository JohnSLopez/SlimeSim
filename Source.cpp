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

int radius = windowHeight / 3;

float currentTime = 0;
float prevTime = 0;
float deltaTime= 0;
# define PI 3.14159265358979323846  /* pi */
bool paused = true;

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

	glfwSetKeyCallback(window, key_callback);

	//Generate agents with random direction and spawn in center of screen
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 2 * PI);

	Agent *agents;
	agents = (Agent*)malloc(numAgents * sizeof(Agent));

	for (int i = 0; i < numAgents; i++)
	{
		Agent temp;

		/*temp.x = windowWidth / 2;
		temp.y = windowHeight / 2;
		temp.direction = distribution(generator);*/

		std::uniform_real_distribution<float> randomDistance(0, radius);
		std::uniform_real_distribution<float> randomAngle(0, 2 * PI);

		int distance = randomDistance(generator);
		float angle = randomAngle(generator);

		temp.x = (windowWidth / 2) + (cos(angle) * distance);
		temp.y = (windowHeight / 2) + (sin(angle) * distance);

		temp.direction = angle + PI;

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

	glClearColor(0, .043137254901960784, .2549019607843137, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		//Handle pausing and closing input
		processInput(window);
		glfwPollEvents();

		if (!paused)
		{
			//get deltaTime
			currentTime = glfwGetTime();
			deltaTime = currentTime - prevTime;
			prevTime = currentTime;

			//clear previous screen
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
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
		}
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_SPACE)
	{
		paused = !paused;
	}
}