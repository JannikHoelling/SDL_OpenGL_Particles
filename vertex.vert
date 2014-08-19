#version 140

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

in vec4 position;
in vec4 velocity;

out vec4 color;

void main()
{
    //Process vertex
	color = velocity;
    gl_Position = projectionMatrix * viewMatrix * position;
}