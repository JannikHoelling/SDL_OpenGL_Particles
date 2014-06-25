#version 330

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
in vec4 position;

void main()
{
    //Process vertex
    gl_Position = projectionMatrix * viewMatrix * position;
}