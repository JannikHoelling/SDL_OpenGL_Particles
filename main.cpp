//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "GLShader.hpp"
#include "cl_helper.hpp"

#include <CL/opencl.h>

const float PI = 3.1415927410125732421875f;
const float G = 6.67384E-11f;
const float MASS = 10E4f;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const bool useAntialiasing = true;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

bool initGLObjects();
bool initCL();

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();
void clUpdate();

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

//Program settings

const int particleCount = 256 * 64 * 1;

//Graphics program
GLuint gProgramID = 0;
GLuint pos_v_ID = 0;
GLuint vel_v_ID = 0;

glm::vec4 pos[particleCount];
glm::vec4 vel[particleCount];

//glm::vec4 *pos = new glm::vec4[particleCount];
//glm::vec4 *vel = new glm::vec4[particleCount];

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
GLint projectionMatrixLocation;
GLint viewMatrixLocation;

GLint posLocation;
GLint velLocation;

cl_float timeScale = 0.5f;

glm::vec3 camPos = glm::vec3(0.0f, 0.25f, 1.0f);

cl_command_queue queue;
cl_mem mem_p;
cl_mem mem_v;
cl_kernel kernel;

bool useCL = true;

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
		if(useAntialiasing) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
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

				std::cout << "OpenGL Vendor:" << vendor << std::endl;
				std::cout << "OpenGL Renderer:" << renderer << std::endl;
				std::cout << "OpenGL Version:" << version << std::endl;

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
				if( !initGLObjects() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}
				//Initialize OpenGL
				if( !initCL() )
				{
					printf( "Unable to initialize OpenCL!\n" );
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
	gProgramID = glCreateProgram();

	GLint vertexShader = loadVertexShader("vertex.vert");
	GLint fragmentShader = loadFragmentShader("fragment.frag");

	gProgramID = loadShader(vertexShader, fragmentShader);

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
		p = glm::vec4(cos(angle) * radius, 0.25f , sin(angle) * radius, 0);
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
	projectionMatrixLocation = glGetUniformLocation(gProgramID, "projectionMatrix");
	viewMatrixLocation = glGetUniformLocation(gProgramID, "viewMatrix");

	posLocation = glGetAttribLocation(gProgramID, "position"); 
	velLocation = glGetAttribLocation(gProgramID, "velocity"); 

	if(projectionMatrixLocation == -1 || viewMatrixLocation == -1) {
		printf("Matrix Locations not found in program!\n");
		return false;
	}

	return true;
}

bool initCL() {
	cl_int err;

	cl_platform_id platform_id;
	cl_program program;
	cl_device_id device_id = cl_getDevice(platform_id);

	#ifdef _WIN32 || _WIN64
		cl_context_properties props[] =
		{
			//OpenCL platform
			CL_CONTEXT_PLATFORM,	(cl_context_properties)	platform_id,
			//OpenGL context
			CL_GL_CONTEXT_KHR,		(cl_context_properties)	wglGetCurrentContext(),
			//HDC used to create the OpenGL context
			CL_WGL_HDC_KHR,			(cl_context_properties)	wglGetCurrentDC(),
			0
		};

		cl_context context = clCreateContext(props, 1, &device_id,NULL, NULL, &err);
		cl_Err(err, "Create context");

		queue = clCreateCommandQueue(context, device_id,CL_QUEUE_PROFILING_ENABLE, &err);
		cl_Err(err, "Create command queue");
	
		mem_p = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, pos_v_ID, &err);
		cl_Err(err, "Create from GL Buffer");

		mem_v = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, vel_v_ID, &err);
		cl_Err(err, "Create from GL Buffer");

		std::string kernelStr = readFile("kernel.cl");
		const char *kernelSrc = kernelStr.c_str();

		program = clCreateProgramWithSource(context, 1, &kernelSrc, NULL, &err); 
		cl_Err(err, "Create Program from Source");

		err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL); 
		cl_Err(err, "Build program");

		kernel = clCreateKernel(program, "simulation", &err);
		cl_Err(err, "Create Kernel");
	#else
		printf("Only Windows supported at this point!\n");
		return false;
	#endif

	//clInfo();

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
		cl_int err;
		err = clEnqueueAcquireGLObjects(queue, 1, &mem_p, NULL, NULL, NULL);
		cl_Err(err, "Acquire GL Objects");

		err = clEnqueueAcquireGLObjects(queue, 1, &mem_v, NULL, NULL, NULL);
		cl_Err(err, "Acquire GL Objects");

		err = clEnqueueReadBuffer(queue, mem_p, CL_TRUE, NULL, particleCount * sizeof(glm::vec4) , &pos, NULL, NULL, NULL); 
		cl_Err(err, "Cl Read");

		err = clEnqueueReadBuffer(queue, mem_v, CL_TRUE, NULL, particleCount * sizeof(glm::vec4) , &vel, NULL, NULL, NULL); 
		cl_Err(err, "Cl Read");

		err = clEnqueueReleaseGLObjects(queue, 1, &mem_p, NULL, NULL, NULL);
		cl_Err(err, "Release GL Objects");

		err = clEnqueueReleaseGLObjects(queue, 1, &mem_v, NULL, NULL, NULL);
		cl_Err(err, "Release GL Objects");
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

void clUpdate() {
	glFinish();

	cl_int err;
	err = clEnqueueAcquireGLObjects(queue, 1, &mem_p, NULL, NULL, NULL);
	cl_Err(err, "Acquire GL Objects");

	err = clEnqueueAcquireGLObjects(queue, 1, &mem_v, NULL, NULL, NULL);
	cl_Err(err, "Acquire GL Objects");

	err = clSetKernelArg(kernel, 0, sizeof(mem_p), &mem_p);
	cl_Err(err, "Set Kernel Arguments");

	err = clSetKernelArg(kernel, 1, sizeof(mem_v), &mem_v);
	cl_Err(err, "Set Kernel Arguments");
	
	int iterations = 4;
	float t = timeScale/iterations;
	err = clSetKernelArg(kernel, 2, sizeof(cl_float), &t);
	cl_Err(err, "Set Kernel Arguments");

	const size_t workSize = particleCount;
	const size_t localWorkSize = 256;

	cl_event e;
	for(size_t i = 0; i < iterations; i++) {
		

		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &workSize, NULL, NULL, NULL, &e);
		cl_Err(err, "Enqueue Kernel");

		err = clFinish(queue);
		cl_Err(err, "Cl Finish");

		
	}
	/*clWaitForEvents(1 , &e);
	cl_ulong time_start, time_end;
	double total_time;
	clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(e, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
	total_time = time_end - time_start;
	printf("Execution time in milliseconds = %0.5f ms\n", (total_time / 1000000.0) );*/

	//err = clEnqueueReadBuffer(queue, mem_p, CL_TRUE, NULL, particleCount * sizeof(glm::vec4) , &pos, NULL, NULL, NULL); 
	//cl_Err(err, "Cl Read");

	//err = clEnqueueReadBuffer(queue, mem_v, CL_TRUE, NULL, particleCount * sizeof(glm::vec4) , &vel, NULL, NULL, NULL); 
	//cl_Err(err, "Cl Read");

	err = clEnqueueReleaseGLObjects(queue, 1, &mem_p, NULL, NULL, NULL);
	cl_Err(err, "Release GL Objects");

	err = clEnqueueReleaseGLObjects(queue, 1, &mem_v, NULL, NULL, NULL);
	cl_Err(err, "Release GL Objects");
}

void render()
{
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT );
	
	//Render quad
	if( gRenderQuad )
	{
		//Bind program
		glUseProgram( gProgramID );

		//Apply projection and view matrices
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

		//Enable vertex arrays
		glEnableClientState( GL_VERTEX_ARRAY );

		//Set vertex data and draw
		glBindBuffer( GL_ARRAY_BUFFER, pos_v_ID );
		glVertexPointer( 3, GL_FLOAT, sizeof(glm::vec4), NULL );

		//glDrawElements( GL_POINTS, particleCount, GL_UNSIGNED_INT, NULL );
		glDrawArrays(GL_POINTS, 0, particleCount);

		GLenum error = glGetError();
		if( error != GL_NO_ERROR )
		{
			printf( "Error ! %s\n");
		}

		//Disable vertex arrays
		glDisableClientState( GL_VERTEX_ARRAY );

		//Unbind program
		glUseProgram( NULL );
	}
}

void close()
{
	cl_int err = clReleaseMemObject(mem_p);
	cl_Err(err, "Release Mem Object");

	err = clReleaseMemObject(mem_v);
	cl_Err(err, "Release Mem Object");

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

			if(useCL) {
				clUpdate();
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

	return 0;
}