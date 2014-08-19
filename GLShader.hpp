#ifndef GLSHADER_H
#define GLSHADER_H

#include "GL/glew.h"
#include <string>

std::string readFile(const char *filePath);
GLuint loadVertexShader(const char *vertex_path);
GLuint loadFragmentShader(const char *fragment_path);
GLuint loadComputeShader(const char *compute_path);
GLuint loadProgram(GLuint vertex, GLuint fragment);
GLuint  loadComputeProgram(GLuint compute);

void glCheckError(GLenum error, std::string location);

#endif