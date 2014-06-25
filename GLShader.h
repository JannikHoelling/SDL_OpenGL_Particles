#ifndef GLSHADER_H
#define GLSHADER_H

#include "GL/glew.h"

GLuint loadVertexShader(const char *vertex_path);
GLuint loadFragmentShader(const char *fragment_path);
GLuint loadShader(GLuint vertex, GLuint fragment);

#endif