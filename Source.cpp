#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VertexBuffer.h"
#include <iostream>
#include "Shader.h"
#include "ComputeShader.h"

int windowWidth = 1920;
int windowHeight = 1080;
//float deltaTime;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

struct Agent
{
	struct position
	{
		float x = 0.0f;
		float y = 0.0f;
	};
	position agentPosition;

	float direction = 0.0f;

	Agent(float x, float y, float inputDirection) 
	{
		agentPosition.x = x;
		agentPosition.y = y;
		direction = inputDirection;
	}
};

Agent agents[] =
{
	Agent(-0.5f, -0.5f, 0.0f),
	Agent(0.5f, -0.5f, 0.0f),
	Agent(0.0f, 0.5f, 0.0f)
};

int main()
{
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	//Initialize Vertex Array Object
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Initialize Vertex Buffers
	VertexBuffer agentBuffer(agents, sizeof(agents));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Create shaders
	Shader vfSlimeShader("slimeShader.vert", "slimeShader.frag");
	ComputeShader cSlimeShader("slimeShader.comp");

	vfSlimeShader.use();
	vfSlimeShader.setInt("tex", 0);

	//Create Textures
	const unsigned int TEXTURE_WIDTH = 1920, TEXTURE_HEIGHT = 1080;
	unsigned int texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glPointSize(10);
	glBindVertexArray(0);

	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		//rendering commands here
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		cSlimeShader.use();
		glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glClear(GL_COLOR_BUFFER_BIT);
		vfSlimeShader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 3);

		//call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}