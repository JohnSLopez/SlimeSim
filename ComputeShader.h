#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ComputeShader
{
public:
	unsigned int ID;

	ComputeShader(const char* computePath)
	{
		std::string cShaderCode;
		std::ifstream cShaderFile;

		cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			cShaderFile.open(computePath);

			std::stringstream cShaderStream;
			cShaderStream << cShaderFile.rdbuf();

			cShaderFile.close();
			cShaderCode = cShaderStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
		}

		const char* shaderCode = cShaderCode.c_str();

		unsigned int compute;

		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &shaderCode, NULL);
		glCompileShader(compute);
		checkCompileErrors(compute, "COMPUTE");

		//Shader Program
		ID = glCreateProgram();
		glAttachShader(ID, compute);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		glDeleteShader(compute);
	}

	void use()
	{
		glUseProgram(ID);
	}
	void setUInt(const std::string& name, int value) const
	{
		glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

private:
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << std::endl << infoLog << std::endl << "-------------------------------------------";
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << std::endl << infoLog << std::endl << "-------------------------------------------";
			}
		}
	}
};