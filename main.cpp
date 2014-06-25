//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLShader.hpp"

typedef struct { 
	GLfloat x, y, dx, dy;
} Vertex;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes rendering program and clear color
bool initGL();

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Program settings
const int vertexcount = 128;

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

//GLfloat vertexData[vertexcount*2];
std::vector<Vertex> vertices;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
GLint projectionMatrixLocation;
GLint viewMatrixLocation;

bool init()
{
	//Initialization flag
	bool success = true;

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
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

		//Create window
		gWindow = SDL_CreateWindow( "SDL OpenGL Particles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

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
				//Initialize GLEW
				glewExperimental = GL_TRUE; 
				GLenum glewError = glewInit();
				if( glewError != GLEW_OK )
				{
					printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
				}

				//Use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//Initialize OpenGL
				if( !initGL() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	//Generate program
	gProgramID = glCreateProgram();

	GLint vertexShader = loadVertexShader("vertex.vert");
	GLint fragmentShader = loadFragmentShader("fragment.frag");

	gProgramID = loadShader(vertexShader, fragmentShader);

	//Initialize clear color
	glClearColor( 0.f, 0.f, 0.f, 1.f );

	//VBO data
	//IBO data
	GLuint indexData[vertexcount];

	for(int i=0 ; i < vertexcount; i++) {
		Vertex v;

		float radius = 0.5f;

		float angle = 1.0f/vertexcount * i * M_PI * 2;

		v.x = cos(angle) * radius;
		v.y = sin(angle) * radius;

		float dx = v.y;
		float dy = -v.x;

		float length = sqrt(dx * dx + dy * dy);
		float force = sqrt((6.67384E-11 * 10E03) / length);

		dx /= length;
		dy /= length;

		dx *= force;
		dy *= force;

		v.dx = dx;
		v.dy = dy;

		vertices.push_back(v);
		indexData[i] = i;
	}

	//Create VBO
	glGenBuffers( 1, &gVBO );
	glBindBuffer( GL_ARRAY_BUFFER, gVBO );
	glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_COPY );

	//Create IBO
	glGenBuffers( 1, &gIBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, vertexcount * sizeof(GLuint), indexData, GL_STATIC_DRAW );
	
	//Create projection and view matrices
	projectionMatrix = glm::perspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.f);
	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.0f, -1.f));

	//Get locations of view and projection matrices
	projectionMatrixLocation = glGetUniformLocation(gProgramID, "projectionMatrix");
	viewMatrixLocation = glGetUniformLocation(gProgramID, "viewMatrix");

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
}

void update()
{
	//printf("UDPATE \n");
	
	for(int i=0 ; i < vertexcount; i++) {

		float dx = 0.0f - vertices[i].x;
		float dy = 0.0f - vertices[i].y;

		float r = sqrt(dx * dx + dy * dy);

		float force = (6.67384E-11 * 10E03) / (r*r);

		vertices[i].dx += dx * force / r;
		vertices[i].dy += dy * force / r;

		vertices[i].x += vertices[i].dx;
		vertices[i].y += vertices[i].dy;
	}

	// Update VBO
	glBindBuffer( GL_ARRAY_BUFFER, gVBO );
	glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_COPY );
}

void render()
{
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT );
	
	//Render quad
	if( gRenderQuad )
	{
		glPointSize( 3.0);

		//Bind program
		glUseProgram( gProgramID );

		//Apply projection and view matrices
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

		//Enable vertex arrays
		glEnableClientState( GL_VERTEX_ARRAY );

		//Set vertex data and draw
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glVertexPointer( 2, GL_FLOAT, sizeof(Vertex), NULL );
		glDrawElements( GL_POINTS, vertexcount, GL_UNSIGNED_INT, NULL );

		//Disable vertex arrays
		glDisableClientState( GL_VERTEX_ARRAY );

		//Unbind program
		glUseProgram( NULL );
	}
}

void close()
{
	//Deallocate program
	glDeleteProgram( gProgramID );

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

			//Update Program
			update();

			//Render quad
			render();
			
			//Update screen
			SDL_GL_SwapWindow( gWindow );
		}
		
		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	close();

	return 0;
}