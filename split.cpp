#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader/shader.h"
#include "camera/camera.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <chrono>

#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "scene/scene.h"
#include "renderable/renderable.h"
#include "origin/origin.h"
#include "model/model.h"
#include "model/cube.h"

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
		camera->BinarySensitivity = 2.0f;

		world.reset( new GLScene() );
		world->shaders["basic_textured"].reset( new _shader("assets/basic_colored.vert","assets/basic_colored.frag") );
		world->models["cube"].reset( new myCube() );
		world->models["cube"]->shader = world->shaders["basic_textured"];
		world->renderables["cube_0"].reset( new Renderable( world->models["cube"] ) );

		origin.reset( new Origin() );

		glLineWidth(2.0f);

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		//glEnable(GL_TEXTURE_2D);

		clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

		//cubes.push_back(std::make_shared<myCube>(new myCube()));

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
		flags.showDemoMenu = false;

		start = std::chrono::steady_clock::now();

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		SDL_GL_SetSwapInterval(0);
		SDL_ShowWindow(window);

		flags.selectClosest = false;
		fontSize = 1.0f;

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
		bool addCube = false;
		bool clearCubes = false;
		// Check for any inputs from the user
		pollInput();

		// Get amount of time since last frame in milliseconds, used to determine how much movement is necessary
		float deltaTime = float((std::chrono::steady_clock::now() - start).count()) / 1000000.0f;
		start = std::chrono::steady_clock::now();

		// Calculate new matrices given the time since the last frame
		camera->InputUpdate(deltaTime);

		// Projection and view are the same per model because they are affected by the camera
		glm::mat4 projection = camera->GetProjectionMatrix(0.01f,100.0f);
		glm::mat4 view = camera->GetViewMatrix();

		glDepthMask( GL_TRUE );

		// Set clear color, clear screen and depth
		glClearColor(clear_color.Value.x,clear_color.Value.y,clear_color.Value.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		std::list<std::pair<std::shared_ptr<Renderable>, float> > sorted;
		std::pair<std::shared_ptr<Renderable>, float> closestHovered
			= std::make_pair(nullptr, 1000.0f);

		for (auto &cube : world->renderables)
		{
			cube.second->flags.isHovered = cube.second->raycastAABB(camera->Position, camera->Front);
			if (cube.second->flags.isHovered)
			{
				float dis = cube.second->distance(camera->Position);
				if ( dis < closestHovered.second )
				{
					closestHovered = std::make_pair(cube.second,dis);
				}
			}
		}

		if (closestHovered.second < 1000.0f)
		{
			closestHovered.first->flags.isClosest = true;

			if ( flags.selectClosest )
			{
				closestHovered.first->flags.isSelected = ! closestHovered.first->flags.isSelected;
				flags.selectClosest = false;
			}
		}

		for (auto &cube : world->renderables)
		{
			if (cube.second->alpha == 1.0f && !cube.second->flags.isClosest)
				cube.second->render(projection,view,deltaTime);
			else
			{
				bool isInserted = false;
				float dis = cube.second->distance(camera->Position);
				for (auto it = sorted.begin(); it != sorted.end(); it++)
				{
					if ( dis > it->second )
					{
						sorted.insert( it, std::make_pair(cube.second,dis) );
						isInserted = true;
						break;
					}
				}

				if ( !isInserted )
				{
					sorted.emplace_back(std::make_pair(cube.second,dis));
				}
			}
		}

		origin->render(projection, view, 1.f);

		//glDepthMask( GL_FALSE );
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (auto &cube : sorted)
		{
			cube.first->render(projection,view,deltaTime);
		}
		glDisable(GL_BLEND);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if ( SDL_GetRelativeMouseMode() == SDL_FALSE )
		{
			if ( flags.showDemoMenu )
				ImGui::ShowDemoWindow(&flags.showDemoMenu);

			ImGui::Begin("Menu",(bool*)__null,ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);// Create a window called "Hello, world!" and append into it.

			ImGui::Text("Press ESCAPE to toggle menu");               // Display some text (you can use a format strings too)

			if (ImGui::Button("Fullscreen"))
			{
				// Check if my fullscreen flag is set, set it to opposite
				bool isF = SDL_GetWindowFlags(window) & myFFlag;
				SDL_SetWindowFullscreen(window, isF ? 0 : myFFlag);
			}
			ImGui::SameLine();
			if (ImGui::Button("DemoMenu")) flags.showDemoMenu = !flags.showDemoMenu;
			ImGui::SameLine();
			if (ImGui::Button("Quit")) flags.doLoop = false;
			if ( ImGui::Button("New Cube") ) addCube = true;
			ImGui::SameLine();
			if ( ImGui::Button("Clear Cubes") ) clearCubes = true;
			ImGui::ColorEdit3("Background Color", (float*)&clear_color); // Edit 3 floats representing a color
			
			ImGui::DragFloat("Font Size", &fontSize, 0.01f);
			ImGui::SetWindowFontScale(fontSize);

			ImGui::Text("Cubes: %ld, Sorted: %ld", world->renderables.size(), sorted.size());

			for (auto &cube : world->renderables)
			{
				std::string name = cube.first;
				if ( cube.second->flags.isSelected && ImGui::TreeNode(name.c_str()) )
				{
					ImGui::SliderFloat("Alpha", &cube.second->alpha, 0.0f, 1.0f);

					ImGui::DragFloat("Spin X", &cube.second->spinAxis.x, 0.01f);
					ImGui::DragFloat("Spin Y", &cube.second->spinAxis.y, 0.01f);
					ImGui::DragFloat("Spin Z", &cube.second->spinAxis.z, 0.01f);

					ImGui::DragFloat("Rot X", &cube.second->rotAxis.x);
					ImGui::DragFloat("Rot Y", &cube.second->rotAxis.y);
					ImGui::DragFloat("Rot Z", &cube.second->rotAxis.z);

					ImGui::DragFloat("X", &cube.second->trans.x, 0.01f);
					ImGui::DragFloat("Y", &cube.second->trans.y, 0.01f);
					ImGui::DragFloat("Z", &cube.second->trans.z, 0.01f);

					ImGui::TreePop();
				}
			}

			ImGui::End();
		}

		ImGui::Begin("Overlay",(bool*)__null,	ImGuiWindowFlags_NoDecoration |
												ImGuiWindowFlags_NoBackground |
												ImGuiWindowFlags_NoInputs |
												ImGuiWindowFlags_NoNav );
		ImGui::SetWindowPos({0,0});

		ImGui::SetWindowFontScale(fontSize);

		ImGui::SetWindowSize( ImGui::GetIO().DisplaySize );

		ImGui::Text("FPS: %.3f", 1000.0f/deltaTime);
		ImGui::Text("Camera: %.3f,%.3f,%.3f", camera->Position.x, camera->Position.y, camera->Position.z);
		ImGui::Text("Camera Direction: %.3f,%.3f,%.3f", camera->Front.x, camera->Front.y, camera->Front.z);
		ImGui::Text("Yaw: %.3f, Pitch: %.3f", camera->Yaw, camera->Pitch);
		ImGui::Text("Zoom: %.3f", camera->Zoom);

		std::string crosshair = "   |   \n"\
								"---+---\n"\
								"   |   ";
		auto windowSize = ImGui::GetWindowSize();
		auto textSize   = ImGui::CalcTextSize(crosshair.c_str());

		ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);
		ImGui::Text(crosshair.c_str());

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap the internal framebuffer to the screen
		SDL_GL_SwapWindow(window);

		glClearColor(clear_color.Value.x,clear_color.Value.y,clear_color.Value.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if ( addCube ) world->renderables["cube_" + std::to_string(world->renderables.size())] = std::make_shared<Renderable>( new Renderable( world->models["cube"] ));
		if ( clearCubes ) world->renderables.clear();

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

				case SDL_MOUSEBUTTONDOWN:
					for (auto &cube : world->renderables)
						if ( cube.second->flags.isHovered )
							flags.selectClosest = true;
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
		bool showDemoMenu;
		bool selectClosest;
	} flags;

	int errorval;
	SDL_Window *window;
	SDL_GLContext glcontext;

	float fontSize;

	ImColor clear_color;

	std::chrono::_V2::steady_clock::time_point start;

	std::shared_ptr<GLScene> world;
	std::unique_ptr<Origin> origin;
	std::shared_ptr<Camera> camera;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
