#ifndef GLSHADER_H
#define GLSHADER_H

#include "GL/glew.h"
#include <string>

std::string readFile(const char *filePath);
GLuint loadVertexShader(const char *vertex_path);
GLuint loadFragmentShader(const char *fragment_path);
GLuint loadShader(GLuint vertex, GLuint fragment);

#endif