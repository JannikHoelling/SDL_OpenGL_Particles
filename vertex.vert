#version 430

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

struct Particle {
    vec4 pos;
    vec4 vel;
};

//layout(location = 0)  in Particle particle;
layout(location = 0)  in vec4 position;
layout(location = 1)  in vec4 velocity;

//out vec4 color;

void main()
{
	//color = normalize(velocity);
    gl_Position = projectionMatrix * viewMatrix * position;
}
