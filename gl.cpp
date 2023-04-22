#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader/shader.h"
#include "camera/camera.h"
#include <iostream>
#include <memory>
#include <chrono>

#include "assets/rawcube.h"

#define ENABLE_MCAP true
#define ENABLE_AA true
#define WWIDTH 640
#define WHEIGHT 480

class gldemo
{
public:
	gldemo()
	{
		flags.doLoop = false;
		errorval = 0;
		if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) < 0 )
		{
			errorval = 1;
			return;
		}

		// Use OpenGL v3.3 core
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		if (ENABLE_AA)
		{
			// Enable 8x Antialiasing
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
			glEnable(GL_MULTISAMPLE);
		}

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

		window = SDL_CreateWindow(	"gldemo",
									SDL_WINDOWPOS_UNDEFINED,
									SDL_WINDOWPOS_UNDEFINED,
									WWIDTH,
									WHEIGHT,
									SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_RESIZABLE );
		if( window == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            return;
        }

		glcontext = SDL_GL_CreateContext( window );
		if( glcontext == NULL )
		{
			printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
			return;
		}

		if (SDL_SetRelativeMouseMode(ENABLE_MCAP ? SDL_TRUE : SDL_FALSE) != 0 )
		{
			std::cout << "Couldn't capture mouse! SDL Error: " << SDL_GetError() << std::endl;
			return;
		}
		SDL_SetWindowGrab(window,SDL_TRUE);

		camera.reset(new Camera(glm::vec3(0.0f,0.5f,5.0f)));
		camera->setViewSize(WWIDTH,WHEIGHT);
		camera->MovementSpeed = 0.01f;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		glGenVertexArrays(1,&VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1,&vertbuff);
		glBindBuffer(GL_ARRAY_BUFFER,vertbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);

		glGenBuffers(1, &colorbuff);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

		shader.load("assets/demo.vert","assets/demo.frag");

		flags.doLoop = true;
	}

	~gldemo()
	{
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(window);
	}

	void runOnce()
	{
		pollInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
		start = std::chrono::steady_clock::now();

		camera->InputUpdate(deltaTime);

		// Projection and view are the same per model because they are affected by the camera
		glm::mat4 projection = camera->GetProjectionMatrix(0.01f,1000.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		// These processes apply to each individual model
		//model = glm::scale(model, glm::vec3(x,y,x));
		//model = glm::translate(model, glm::vec3(x,y,z));
		//model = model * glm::toMat4(glm::quat(w,x,y,z));

		shader.use();
		shader.setMat4("model",model);
		shader.setMat4("view",view);
		shader.setMat4("projection",projection);

		glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
		glVertexAttribPointer(
			0,                  // id
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
		glVertexAttribPointer(
			1,                  // id
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, 12*3);

		glDisableVertexArrayAttrib(VAO, 0);
		glDisableVertexArrayAttrib(VAO, 1);

		SDL_GL_SwapWindow(window);

		return;
	}

	bool isAlive()
	{
		return flags.doLoop;
	}

private:
	void pollInput()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
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
						case SDLK_ESCAPE:
							flags.doLoop = false;
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
					}
				break;

				case SDL_MOUSEMOTION:
					camera->ProcessMouseMovement(e.motion.xrel,e.motion.yrel,true);
				break;

				case SDL_MOUSEWHEEL:
					camera->ProcessMouseScroll(e.wheel.preciseY);
				break;

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
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
