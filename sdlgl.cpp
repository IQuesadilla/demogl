#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader/shader.h"
#include "camera/camera.h"
#include <iostream>
#include <memory>
#include <chrono>

#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include "assets/rawcube.h"

#define FullOnStart false
#define myFFlag SDL_WINDOW_FULLSCREEN_DESKTOP
#define AA_LEVEL 0
#define WWIDTH 640
#define WHEIGHT 480

class gldemo
{
public:
	gldemo()
	{
		// Don't enable the main loop unless init succeeds
		flags.doLoop = false;

		// Initialize Video and Events on SDL, no need to initialize any other subsystems
		if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) < 0 )
		{
			std::cout << "Failed to init SDL! SDL: " << SDL_GetError() << std::endl;
			return;
		}

		// Use OpenGL v3.3 core - GLSL: #version 330 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		#if AA_LEVEL
			// Enable Antialiasing
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, AA_LEVEL);
			glEnable(GL_MULTISAMPLE);
		#endif

		GLenum error = GL_NO_ERROR;

		//Initialize Projection Matrix
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		
		//Check for error
		error = glGetError();
		if( error != GL_NO_ERROR )
		{
			std::cout << "Error initializing OpenGL! " << error << std::endl;
			return;
		}

		//Initialize Modelview Matrix
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if( error != GL_NO_ERROR )
		{
			std::cout << "Error initializing OpenGL! " << error << std::endl;
			return;
		}

		window = SDL_CreateWindow(	"gldemo", 					// Window Title
									SDL_WINDOWPOS_UNDEFINED,	// Starting Global X Position
									SDL_WINDOWPOS_UNDEFINED,	// Starting Global Y Position
									WWIDTH,						// Window Width
									WHEIGHT,					// Window Height
									SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_RESIZABLE );

		// window will be NULL if CreateWindow failed
		if( window == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            return;
        }

		// start OpenGL within the SDL window, returns NULL if failed
		glcontext = SDL_GL_CreateContext( window );
		if( glcontext == NULL )
		{
			printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
			return;
		}

		// This should happen by default, but just to be sure, attach the glcontext to the window
		SDL_GL_MakeCurrent(window, glcontext);

		// Load window icon and set if successfully loaded
		SDL_Surface *icon = SDL_LoadBMP("assets/opengl.bmp");
		if (icon == NULL)
		{
			std::cout << "Failed to load icon image" << std::endl;
		}
		else
		{
			SDL_SetWindowIcon(window, icon);
			SDL_FreeSurface(icon);
		}

		// Create a new camera and set it's default position to (x,y,z)
		// +z is towards you, -z is away from you
		camera.reset(new Camera(glm::vec3(0.0f,0.5f,5.0f)));
		camera->setViewSize(WWIDTH,WHEIGHT);
		camera->MovementSpeed = 0.01f;
		camera->BinarySensitivity = 4.0f;

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Don't render faces that have a normal facing away from the viewport
		glEnable(GL_CULL_FACE);

		// Create a Vertex Array Object to represent the cube
		glGenVertexArrays(1,&VAO);
		glBindVertexArray(VAO);

		// Generate a Vertex Buffer Object to represent the cube's vertices
		glGenBuffers(1,&vertbuff);
		glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);

		// Generate a Vertex Buffer Object to represent the cube's colors
		glGenBuffers(1, &colorbuff);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

		// Set vertices to location 0 - GLSL: layout(location = 0) in vec3 aPos;
		glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
		glVertexAttribPointer(
			0,                  // location
			3,                  // size (per vertex)
			GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
			GL_FALSE,           // is normalized*
			0,                  // stride**
			(void*)0            // array buffer offset
		);

		// Set colors to location 1 - GLSL: layout(location = 1) in vec3 aColor;
		glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
		glVertexAttribPointer(
			1,                  // location
			3,                  // size (per vertex)
			GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
			GL_FALSE,           // is normalized*
			0,                  // stride**
			(void*)0            // array buffer offset
		);

		// Load and compile the basic demo shaders, returns true if error
		if ( shader.load("assets/basic_colored.vert","assets/basic_colored.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << shader.getErrors() << std::endl;
			return;
		}

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		SDL_ShowWindow(window);

		rot = 0.0f;

		// If here, initialization succeeded and loop should be enabled
		flags.doLoop = true;
	}

	~gldemo()
	{
		glDeleteBuffers(1,&colorbuff);
		glDeleteBuffers(1,&vertbuff);
		glDeleteVertexArrays(1,&VAO);
		// Properly shutdown OpenGL and destroy the window
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(window);
	}

	void runOnce()
	{
		// Check for any inputs from the user
		pollInput();

		// Set clear color, clear screen and depth
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get amount of time since last frame, used to determine how much movement is necessary
		float deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		start = std::chrono::steady_clock::now();

		// Calculate new matrices given the time since the last frame
		camera->InputUpdate(deltaTime);

		// Projection and view are the same per model because they are affected by the camera
		glm::mat4 projection = camera->GetProjectionMatrix(0.01f,100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		// These processes apply to each individual model
		//model = glm::scale(model, glm::vec3(x,y,x));
		//model = glm::translate(model, glm::vec3(x,y,z));
		//model = model * glm::toMat4(glm::quat(w,x,y,z));

		model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
			glm::angleAxis(							// Angle axis has two arguments - angle and axis
				glm::radians(rot += deltaTime/60),	// The angle to rotate all the vertices
				glm::vec3(0.0f, 1.0f, 0.0f)));		// Which axis to apply the rotation to and how much - (x,y,z)

		// Use shader "shader" and give it all 3 uniforms
		shader.use();
		shader.setMat4("model",model);				// GLSL: uniform mat4 model;
		shader.setMat4("view",view);				// GLSL: uniform mat4 view;
		shader.setMat4("projection",projection);	// GLSL: uniform mat4 projection;

		// * if normalized is set to GL_TRUE, it indicates that values stored in an integer format are to be mapped
		// * to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted
		// * to floating point. Otherwise, values will be converted to floats directly without normalization. 

		// ** 0 means tightly packed, in this case 0 means OpenGL should automatically calculate size * sizeof(GLFloat) = 12
		// ** no distance from "how many bytes it is from the start of one element to the start of another"

		// Enable location 0 and location 1 in the shader
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		// Draw the cube
		glDrawArrays(GL_TRIANGLES, 0, 12*3);

		// Disable location 0 and location 1
		glDisableVertexArrayAttrib(VAO, 0);
		glDisableVertexArrayAttrib(VAO, 1);

		// Swap the internal framebuffer to the screen
		SDL_GL_SwapWindow(window);

		return;
	}

	bool isAlive()
	{
		// Continue to loop while the flag is set to true
		return flags.doLoop;
	}

private:
	void pollInput()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			// Only check for movement if the mouse is currently captured
			if ( SDL_GetRelativeMouseMode() == SDL_TRUE )
			{
				switch (e.type)
				{
				case SDL_KEYDOWN:
					switch (e.key.keysym.sym)
					{
					case SDLK_w:
						camera->ProcessKeyboard(FORWARD,true);
					break;
					case SDLK_s:
						camera->ProcessKeyboard(BACKWARD,true);
					break;
					case SDLK_d:
						camera->ProcessKeyboard(RIGHT,true);
					break;
					case SDLK_a:
						camera->ProcessKeyboard(LEFT,true);
					break;
					case SDLK_SPACE:
						camera->ProcessKeyboard(UP,true);
					break;
					case SDLK_LSHIFT:
						camera->ProcessKeyboard(DOWN,true);
					break;
					case SDLK_UP:
						camera->ProcessKeyboard(V_UP,true);
					break;
					case SDLK_DOWN:
						camera->ProcessKeyboard(V_DOWN,true);
					break;
					case SDLK_RIGHT:
						camera->ProcessKeyboard(V_RIGHT,true);
					break;
					case SDLK_LEFT:
						camera->ProcessKeyboard(V_LEFT,true);
					break;
					}
				break;

				case SDL_KEYUP:
					switch (e.key.keysym.sym)
					{
					case SDLK_w:
						camera->ProcessKeyboard(FORWARD,false);
					break;
					case SDLK_s:
						camera->ProcessKeyboard(BACKWARD,false);
					break;
					case SDLK_d:
						camera->ProcessKeyboard(RIGHT,false);
					break;
					case SDLK_a:
						camera->ProcessKeyboard(LEFT,false);
					break;
					case SDLK_SPACE:
						camera->ProcessKeyboard(UP,false);
					break;
					case SDLK_LSHIFT:
						camera->ProcessKeyboard(DOWN,false);
					break;
					case SDLK_UP:
						camera->ProcessKeyboard(V_UP,false);
					break;
					case SDLK_DOWN:
						camera->ProcessKeyboard(V_DOWN,false);
					break;
					case SDLK_RIGHT:
						camera->ProcessKeyboard(V_RIGHT,false);
					break;
					case SDLK_LEFT:
						camera->ProcessKeyboard(V_LEFT,false);
					break;
					}
				break;

				case SDL_MOUSEMOTION:
					camera->ProcessMouseMovement(e.motion.xrel,e.motion.yrel,true);
				break;

				case SDL_MOUSEWHEEL:
					camera->ProcessMouseScroll(e.wheel.preciseY);
				break;
				}
			}

			switch (e.type)
			{
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					flags.doLoop = false;
				break;
				case SDLK_f:
					// Check if my fullscreen flag is set, set it to opposite
					SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) & myFFlag) ? 0 : myFFlag);
				break;
				case SDLK_m:
					SDL_SetRelativeMouseMode( SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE );
				break;
				}
			break;

			// On ANY winow related event, make sure the size is updated
			case SDL_WINDOWEVENT:
				int w,h;
				SDL_GL_GetDrawableSize(window,&w,&h);
				camera->setViewSize(w,h);
			break;

			case SDL_QUIT:
				flags.doLoop = false;
			break;
			}
		}
	}

	struct {
		bool doLoop;
	} flags;

	int errorval;
	SDL_Window *window;
	SDL_GLContext glcontext;

	GLuint VAO;
	GLuint vertbuff;
	GLuint colorbuff;

	std::chrono::_V2::steady_clock::time_point start;

	std::shared_ptr<Camera> camera;
	_shader shader;
	float rot;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
