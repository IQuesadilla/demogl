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

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "assets/rawcube.h"

#define FullOnStart false
#define myFFlag SDL_WINDOW_FULLSCREEN_DESKTOP
#define AA_LEVEL 2
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
			// Enable 8x Antialiasing
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

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Don't render faces that have a normal facing away from the viewport
		glEnable(GL_CULL_FACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Create a Vertex Array Object to represent the cube
		glGenVertexArrays(1,&VAO);
		glBindVertexArray(VAO);

		// Generate a Vertex Buffer Object to represent the cube's vertices
		glGenBuffers(1,&vertbuff);
		glBindBuffer(GL_ARRAY_BUFFER,vertbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);

		// Generate a Vertex Buffer Object to represent the cube's colors
		glGenBuffers(1, &colorbuff);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

		// Load and compile the basic demo shaders, returns true if error
		if ( shader.load("assets/demo.vert","assets/demo.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << shader.getErrors() << std::endl;
			return;
		}

		IMGUI_CHECKVERSION();
    	ImGui::CreateContext();
    	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		ImGui::GetIO().IniFilename = NULL;
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
    	ImGui_ImplOpenGL3_Init("#version 330 core");
		ImFontConfig font_cfg;
		font_cfg.OversampleH = 3;
		font_cfg.OversampleV = 3;
		//font_cfg.RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
		ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/font.ttf",15,&font_cfg);
		//ImGui::GetIO().Fonts->Build();

		clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
		cubeSpeed = 0.033f;
		rot = 0.0f;
		alpha = 1.0f;

		start = std::chrono::steady_clock::now();

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		SDL_GL_SetSwapInterval(0);
		SDL_ShowWindow(window);

		// If here, initialization succeeded and loop should be enabled
		flags.doLoop = true;
	}

	~gldemo()
	{
		// Properly shutdown OpenGL and destroy the window
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(window);
	}

	void runOnce()
	{
		// Check for any inputs from the user
		pollInput();

		// Set clear color, clear screen and depth
		glClearColor(clear_color.Value.x,clear_color.Value.y,clear_color.Value.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get amount of time since last frame in milliseconds, used to determine how much movement is necessary
		float deltaTime = float((std::chrono::steady_clock::now() - start).count()) / 1000000.0f;
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

		rot += deltaTime*cubeSpeed;
		model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
			glm::angleAxis(							// Angle axis has two arguments - angle and axis
				glm::radians(rot),					// The angle to rotate all the vertices, converts deg to rad
				glm::vec3(0.0f, 1.0f, 0.0f)));		// Which axis to apply the rotation to and how much - (x,y,z)

		// Use shader "shader" and give it all 3 uniforms
		shader.use();
		shader.setMat4("model",model);				// GLSL: uniform mat4 model;
		shader.setMat4("view",view);				// GLSL: uniform mat4 view;
		shader.setMat4("projection",projection);	// GLSL: uniform mat4 projection;
		shader.setFloat("alpha",alpha);

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

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if ( SDL_GetRelativeMouseMode() == SDL_FALSE )
		{
			ImGui::ShowDemoWindow();
			
			ImGui::Begin("Menu",(bool*)__null,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);// Create a window called "Hello, world!" and append into it.

			ImGui::Text("Press ESCAPE to toggle menu");               // Display some text (you can use a format strings too)
			ImGui::SliderFloat("Cube Speed", &cubeSpeed, 0.001f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);
			ImGui::ColorEdit3("Background Color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Toggle Fullscreen"))
			{
				// Check if my fullscreen flag is set, set it to opposite
				bool isF = SDL_GetWindowFlags(window) & myFFlag;
				SDL_SetWindowFullscreen(window, isF ? 0 : myFFlag);
			}
			ImGui::SameLine();
			if (ImGui::Button("Quit")) flags.doLoop = false;

			ImGui::End();
		}

		ImGui::Begin("Framerate",(bool*)__null,	ImGuiWindowFlags_NoDecoration |
												ImGuiWindowFlags_NoBackground |
												ImGuiWindowFlags_AlwaysAutoResize |
												ImGuiWindowFlags_NoInputs |
												ImGuiWindowFlags_NoNav );
		ImGui::SetWindowPos({0,0});
		ImGui::Text("FPS: %f", 1000.0f/deltaTime);
		ImGui::Text("rot: %d", int(rot) % 360);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
			if ( SDL_GetRelativeMouseMode() == SDL_FALSE )
			{
				ImGui_ImplSDL2_ProcessEvent(&e);
			}
			else
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
				}
			}


			switch (e.type)
			{
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					SDL_SetRelativeMouseMode( SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE );
					//SDL_SetWindowMouseGrab(window, SDL_GetWindowMouseGrab(window) ? SDL_FALSE : SDL_TRUE);
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

	ImColor clear_color;
	float cubeSpeed;

	std::chrono::_V2::steady_clock::time_point start;

	std::shared_ptr<Camera> camera;
	_shader shader;
	float rot, alpha;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
