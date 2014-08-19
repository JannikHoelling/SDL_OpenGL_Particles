//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "GLShader.hpp"

#define WORK_GROUP_SIZE 1024

const float PI = 3.1415927410125732421875f;
const float G = 6.67384E-11f;
const float MASS = 10E4f;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
bool useCL = true;
const bool useAntialiasing = true;
GLfloat timeScale = 0.25f;

const int particleCount = 256 * 256 * 32;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

bool initGLObjects();
//bool initCL();

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();
//void clUpdate();
void computeUpdate();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();
void clInfo();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint shaderProgram = 0;
GLuint computeProgram = 0;
GLuint pos_v_ID = 0;
GLuint vel_v_ID = 0;

glm::vec4 pos[particleCount];
glm::vec4 vel[particleCount];

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
GLint projectionMatrixLocation;
GLint viewMatrixLocation;

GLint posLocation;
GLint velLocation;

glm::vec3 camPos = glm::vec3(0.0f, 0.25f, 1.0f);

bool init()
{
	//Initialization flag
	bool success = true;

	std::cout << particleCount << std::endl;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Use OpenGL 3.1 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		
		//Anti Aliasing
		if(useAntialiasing) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		}

		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		//Create window
		//gWindow = SDL_CreateWindow( "SDL OpenGL Particles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP );
		gWindow = SDL_CreateWindow( "SDL OpenGL Particles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext( gWindow );

			if( gContext == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				const GLubyte* vendor = glGetString(GL_VENDOR);
				const GLubyte* renderer = glGetString(GL_RENDERER);
				const GLubyte* version = glGetString(GL_VERSION);

				const GLubyte* glewVersion = glewGetString(GLEW_VERSION);

				//GLint max;
				//glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &max);

				std::cout << "OpenGL Vendor:" << vendor << std::endl;
				std::cout << "OpenGL Renderer:" << renderer << std::endl;
				std::cout << "OpenGL Version:" << version << std::endl;
				std::cout << "Glew Version:" << glewVersion << std::endl;

				//Initialize GLEW
				glewExperimental = GL_TRUE; 
				GLenum glewError = glewInit();
				if( glewError != GLEW_OK )
				{
					printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );

				}
				else {
					printf("GLEW Init: Success!\n");
				}

				//Use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}
				
				//Initialize OpenGL
				if( !initGLObjects() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGLObjects()
{
	//Generate program
	shaderProgram = glCreateProgram();
	glCheckError(glGetError(), "Create Program");

	GLint vertexShader = loadVertexShader("vertex.vert");
	GLint fragmentShader = loadFragmentShader("fragment.frag");
	GLint computeShader = loadComputeShader("compute.compute");

	shaderProgram = loadProgram(vertexShader, fragmentShader);
	computeProgram = loadComputeProgram(computeShader);
	glCheckError(glGetError(), "Load Shader");

	//Initialize clear color
	glClearColor( 0.f, 0.f, 0.f, 1.f );

	std::cout << "Using " << particleCount << " Particles." << std::endl;

	srand (static_cast <unsigned> (time(0)));

	for(int i=0 ; i < particleCount; i++) {
		glm::vec4 p;
		glm::vec4 v;

		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

		float angle = 1.0f/particleCount * i * PI * 2.0f;

		float radius = 1.0f * (r+0.1f);

		//Position of new Vertex
		p = glm::vec4(cos(angle) * radius, 0.05f , sin(angle) * radius, 0);
		//Direction for normal orbit
		v = glm::vec4(p.z, -p.y , -p.x , 0);
		//Adjust direction with correct force
		float length = glm::length(v);
		float force = sqrtf((G * MASS) / length);
		v /= length;
		v *= force;

		pos[i] = p;
		vel[i] = v;
	}

	//Create VBO
	glGenBuffers( 1, &pos_v_ID );
	glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &pos, GL_DYNAMIC_DRAW );

	//Create velID
	glGenBuffers( 1, &vel_v_ID );
	glBindBuffer( GL_ARRAY_BUFFER, vel_v_ID );
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &vel, GL_DYNAMIC_DRAW );
	
	//Create projection and view matrices
	projectionMatrix = glm::perspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.f);
	//viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.0f, -1.f));

	viewMatrix = glm::translate(viewMatrix, camPos);
	/*viewMatrix = glm::rotate(viewMatrix, 605.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	viewMatrix = glm::rotate(viewMatrix, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));*/
	viewMatrix = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//Get locations of view and projection matrices
	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");

	posLocation = glGetAttribLocation(shaderProgram, "position"); 
	velLocation = glGetAttribLocation(shaderProgram, "velocity"); 

	if(projectionMatrixLocation == -1 || viewMatrixLocation == -1) {
		printf("Matrix Locations not found in program!\n");
		return false;
	}

	return true;
}

void handleKeys( unsigned char key, int x, int y )
{
	//Toggle quad
	if( key == 'q' )
	{
		gRenderQuad = !gRenderQuad;
		
	}
	if( key == 'e' )
	{
		useCL = !useCL;

		// Update VBO
		glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
		glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &pos, GL_DYNAMIC_DRAW ); 

		glBindBuffer( GL_ARRAY_BUFFER, vel_v_ID );
		glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &vel, GL_DYNAMIC_DRAW ); 
	}
}

void update()
{	
	for(int i=0 ; i < particleCount; i++) {

		glm::vec4 d = glm::vec4(0.0f) - pos[i];

		float r = glm::length(d);

		float force = (G * MASS) / (r*r);

		vel[i] += d * force * timeScale / r;

		pos[i] += vel[i] * timeScale;
	}

	// Update VBO
	glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &pos, GL_DYNAMIC_DRAW ); 

	glBindBuffer( GL_ARRAY_BUFFER, vel_v_ID );
	glBufferData( GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec4), &vel, GL_DYNAMIC_DRAW ); 
}

void computeUpdate() {
	glUseProgram(computeProgram);
	glUniform1f(glGetUniformLocation(computeProgram, "timeScale"), timeScale);
	//glUniform4v(glGetUniformLocation(computeProgram, "pos"), pos);
	//glUniform4v(glGetUniformLocation(computeProgram, "vel"), pos);
	//glBindBuffer( GL_ARRAY_BUFFER,  );

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pos_v_ID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vel_v_ID);
	glCheckError(glGetError(), "Dispatch compute shader binding");

    glDispatchCompute(particleCount/WORK_GROUP_SIZE, 1, 1); // 512^2 threads in blocks of 16^2
    glCheckError(glGetError(), "Dispatch compute shader");
	glUseProgram(0);
}

void render()
{
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT );
	
	//Render quad
	if( gRenderQuad )
	{
		//Bind program
		glUseProgram( shaderProgram );

		//Apply projection and view matrices
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

		//Enable vertex arrays
		glEnableClientState( GL_VERTEX_ARRAY );

		// TEST
		//glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
		//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
		//glCheckError(glGetError(), "DrawArrays");
		//glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
		//glBindAttribLocation(shaderProgram, 0, "position");

		//Set vertex data and draw
		glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
		glVertexPointer( 3, GL_FLOAT, sizeof(glm::vec4), NULL );

		//glDrawElements( GL_POINTS, particleCount, GL_UNSIGNED_INT, NULL );
		glDrawArrays(GL_POINTS, 0, particleCount);

		glCheckError(glGetError(), "DrawArrays");

		//Disable vertex arrays
		glDisableClientState( GL_VERTEX_ARRAY );
		
		//Unbind program
		glUseProgram( NULL );
	}
}

void close()
{
	//Deallocate program
	glDeleteProgram( shaderProgram );

	//Destroy window	
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		
		//Enable text input
		SDL_StartTextInput();

		//While application is running
		while( !quit )
		{
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//User requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				//Handle keypress with current mouse position
				else if( e.type == SDL_TEXTINPUT )
				{
					int x = 0, y = 0;
					SDL_GetMouseState( &x, &y );
					handleKeys( e.text.text[ 0 ], x, y );
				}
			}

			if(useCL) {
				computeUpdate();
			}
			else {
				update();
			}

			render();

			
			//Update screen
			SDL_GL_SwapWindow( gWindow );
		}
		
		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	close();

	std::getchar();

	return 0;
}