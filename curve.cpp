#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader/shader.h"
#include "origin/origin.h"
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
		camera->MovementSpeed = 0.005f;
		camera->BinarySensitivity = 4.0f;

		origin.reset( new Origin() );

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Don't render faces that have a normal facing away from the viewport
		//glEnable(GL_CULL_FACE);

		// Create a Vertex Array Object to represent the cube
		glGenVertexArrays(1,&VAO);
		glBindVertexArray(VAO);

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

		const char *funcsrc = "\
		vec3 Mf(float Mt) {\
			float Ma = 5;\
			float Mb = 1;\
			float Mc = 7;\
			return vec3(\
				(Ma - Mb)*cos(Mt) + Mc*cos((Ma/Mb - 1)*Mt) ,\
				(Ma - Mb)*sin(Mt) - Mc*sin((Ma/Mb - 1)*Mt) ,\
				Mt) / 5;\
		}\
		";

		// Load and compile the basic demo shaders, returns true if error
		shader.addVertRaw(std::string(funcsrc));
		if ( shader.load("assets/curve.vert","assets/curve.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << shader.getErrors() << std::endl;
			return;
		}

		// Load and compile the basic demo shaders, returns true if error
		intshader.addVertRaw(std::string(funcsrc));
		if ( intshader.load("assets/integral.vert","assets/integral.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << shader.getErrors() << std::endl;
			return;
		}

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		SDL_ShowWindow(window);

		// If here, initialization succeeded and loop should be enabled
		flags.doLoop = true;
	}

	~gldemo()
	{
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

		float curveLen = glm::pi<float>() * 2.f * 100.0f;
		float tSteps = 200.0f;

		// Projection and view are the same per model because they are affected by the camera
		glm::mat4 projection = camera->GetProjectionMatrix(0.01f,1000.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		glLineWidth(2.0f);
		origin->render(projection, camera->GetRotViewMatrix(), 0.05f);
		origin->render(projection, view, 1.f);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		shader.use();
		shader.setMat4("model",model);				// GLSL: uniform mat4 model;
		shader.setMat4("view",view);				// GLSL: uniform mat4 view;
		shader.setMat4("projection",projection);	// GLSL: uniform mat4 projection;
		shader.setFloat("tScale", 1.f / tSteps );
		shader.setVec3("colormask", glm::vec3(1.0f,1.0f,1.0f) * tSteps);
		shader.setFloat("alpha", 1.0f );
		shader.setInt("startID",-curveLen/2.0f);

		// Draw the cube
		glLineWidth(5.0f);
		glDrawArrays(GL_LINE_STRIP, 0, curveLen*tSteps + 2.0f);

		intshader.use();
		intshader.setMat4("model",model);				// GLSL: uniform mat4 model;
		intshader.setMat4("view",view);				// GLSL: uniform mat4 view;
		intshader.setMat4("projection",projection);	// GLSL: uniform mat4 projection;
		intshader.setInt("startID",-curveLen/2.0f);
		intshader.setFloat("alpha", 0.0f );
		intshader.setFloat("tScale", 1.f / tSteps );

		glLineWidth(1.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, curveLen*tSteps*4 + 4);

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
					// Check if currently capturing mouse, set it to opposite
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

	std::shared_ptr<Origin> origin;

	std::shared_ptr<Camera> camera;
	_shader shader, intshader;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
