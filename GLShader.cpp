#include "GLShader.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

void printProgramLog( GLuint program )
{
	//Make sure name is shader
	if( glIsProgram( program ) )
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		char* infoLog = new char[ maxLength ];
		
		//Get info log
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}
		
		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a program\n", program );
	}
}

void printShaderLog( GLuint shader )
{
	//Make sure name is shader
	if( glIsShader( shader ) )
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		char* infoLog = new char[ maxLength ];
		
		//Get info log
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a shader\n", shader );
	}
}


bool loadVertexShader(GLuint program, const char *shaderSource) 
{
	bool success = true;

	//Create vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//Set vertex source
	glShaderSource(vertexShader, 1, &shaderSource, NULL);

	//Compile vertex source
	glCompileShader( vertexShader );

	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if( vShaderCompiled != GL_TRUE )
	{
		printf("Unable to compile vertex shader %d!\n", vertexShader);
		printShaderLog(vertexShader);
        success = false;
	}
	else {
		printf("Loaded Vertex Shader correctly!\n");
		glAttachShader(program, vertexShader);
	}

	return success;
}

bool loadFragmentShader(GLuint program, const char *shaderSource) 
{
	bool success = true;

	//Create fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//Set fragment source
	glShaderSource(fragmentShader, 1, &shaderSource, NULL);

	//Compile fragment source
	glCompileShader( fragmentShader );

	//Check fragment shader for errors
	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if( fShaderCompiled != GL_TRUE )
	{
		printf( "Unable to compile fragment shader %d!\n", fragmentShader);
		printShaderLog(fragmentShader);
		success = false;
	}
	else {
		printf("Loaded Fragment Shader correctly!\n");
		glAttachShader(program, fragmentShader);
	}
	return success;
}

GLuint LoadShader(const char *vertex_path, const char *fragment_path)
{
	GLuint program = glCreateProgram();

    // Read shaders
    std::string vertShaderStr = readFile(vertex_path);
    std::string fragShaderStr = readFile(fragment_path);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

	loadVertexShader(program, vertShaderSrc);
	loadFragmentShader(program, fragShaderSrc);

	//Link program
	glLinkProgram(program);

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(program, GL_LINK_STATUS, &programSuccess);
	if(programSuccess != GL_TRUE)
	{
		printf("Error linking program %d!\n", program);
		printProgramLog(program);
	}
	else {
		printf("Linked Program correctly!\n");
	}

    return program;
}