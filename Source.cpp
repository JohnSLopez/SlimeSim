#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VertexBuffer.h"
#include <iostream>
#include "Shader.h"

int windowWidth = 1920;
int windowHeight = 1080;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

//struct position
//{
//	float x = 0.0f;
//	float y = 0.0f;
//};

struct Agent
{
	struct position
	{
		float x = 0.0f;
		float y = 0.0f;
	};
	position agentPosition;

	//may need to clamp (0,360)
	float heading = 0.0f;

	Agent(float x, float y, float inputHeading) 
	{
		agentPosition.x = x;
		agentPosition.y = y;
		heading = inputHeading;
	}
};

Agent agents[] =
{
	Agent(-0.5f, -0.5f, 0.0f),
	Agent(0.5f, -0.5f, 0.0f),
	Agent(0.0f, 0.5f, 0.0f)
};

//float vertices[] = 
//{
//	-0.5f, -0.5f, 0.0f,
//	 0.5f, -0.5f, 0.0f,
//	 0.0f,  0.5f, 0.0f
//};

int main()
{
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
	VertexBuffer VBO(agents, sizeof(agents));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Agent), (void*)0);
	glEnableVertexAttribArray(0);

	//Create main shader
	Shader slimeShader("slimeShader.vert", "slimeShader.frag");

	glPointSize(10);
	//glBindVertexArray(0);

	//main render loop
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		//rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		slimeShader.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 3);
		//call events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glDeleteShader(vertexShader);
	//glDeleteShader(fragShader);
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