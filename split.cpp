#define GL_GLEXT_PROTOTYPES 1

#include "SDL.h"
#include <SDL_opengl.h>
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <array>
#include <chrono>
#include <numeric>

#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "scene/imfilebrowser.h"

#include "scene/scene.h"
#include "scene/collada.h"
#include "renderable/renderable.h"
#include "origin/origin.h"
#include "model/model.h"
#include "model/cube.h"
#include "model/window.h"
#include "model/blank.h"

#include "assets/rawcube.h"

#define FullOnStart false
#define myFFlag SDL_WINDOW_FULLSCREEN_DESKTOP
#define AA_LEVEL 8
#define WWIDTH 1280
#define WHEIGHT 720

class gldemo
{
public:
	gldemo()
	{
		// Don't enable the main loop unless init succeeds
		flags.doLoop = false;

		start = std::chrono::steady_clock::now();

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
		#endif
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

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

		#if AA_LEVEL
			glEnable(GL_MULTISAMPLE);
		#endif

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

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		world.reset( new COLLADAScene("assets/cube.dae") );
		//((COLLADAScene*)world.get())->parse();
		world->shaders["basic_textured"].reset( new _shader("assets/basic_textured.vert","assets/basic_textured.frag") );
		world->models["blank"].reset( new Blank() );
		world->models["cube"].reset( new myCube() );
		world->models["cube"]->shader = world->shaders["basic_textured"];
		world->models["window"].reset( new Window() );
		world->models["window"]->shader = world->shaders["basic_textured"];

		/*for (auto &model : world->models)
			if (model.second->shader == nullptr)
			{
				std::cout << "Set default shader for \"" << model.first << '"' << std::endl;
				model.second->shader.reset(&world->AABBShader);
			}*/

		origin.reset( new Origin() );
		cursor.reset( new Origin() );

		clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

		IMGUI_CHECKVERSION();
    	ImGui::CreateContext();
    	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		ImGui::GetIO().IniFilename = NULL;
		ImGui::StyleColorsDark();
		ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
    	ImGui_ImplOpenGL3_Init("#version 330 core");
		ImFontConfig font_cfg;
		font_cfg.OversampleH = 2;
		font_cfg.OversampleV = 2;
		//font_cfg.RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
		font.reset(ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/font.ttf",15,&font_cfg));
		flags.showDemoMenu = false;
		flags.showObjectManager = false;

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "INIT Error:" << err << std::endl;
        }

		GLfloat range[2] = {0.0f,0.0f};
		glGetFloatv(GL_LINE_WIDTH_RANGE, range);
		std::cout << "Line Width: " << range[0] << ", " << range[1] << std::endl;
		glGetFloatv(GL_POINT_SIZE_RANGE, range);
		std::cout << "Point Size: " << range[0] << ", " << range[1] << std::endl;

		int depthBufferSize;
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthBufferSize);
		std::cout << "Allocated Depth Buffer Size: " << depthBufferSize << std::endl;

		fontSize = 1.0f;

		for (auto &x : fps_array)
			x = 1.0f;
		fps_array_it = fps_array.begin();

		if (SDL_GL_SetSwapInterval(-1) < 0)
			SDL_GL_SetSwapInterval(1);
		SDL_ShowWindow(window);

		std::cout << "Setup Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "mS" << std::endl;
		start = std::chrono::steady_clock::now();

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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// Check for any inputs from the user
		pollInput();

		// Get amount of time since last frame in milliseconds, used to determine how much movement is necessary
		float deltaTime = float((std::chrono::steady_clock::now() - start).count()) / 1000000.0f;
		start = std::chrono::steady_clock::now();

		// Calculate new matrices given the time since the last frame
		camera->InputUpdate(deltaTime);

		// Projection and view are the same per model because they are affected by the camera
		//glMatrixMode(GL_PROJECTION);
		glm::mat4 projection = camera->GetProjectionMatrix(0.5f,86.7f);
		glm::mat4 view = camera->GetViewMatrix();

		glDepthMask( GL_TRUE );

		// Set clear color, clear screen and depth
		glClearColor(clear_color.Value.x,clear_color.Value.y,clear_color.Value.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (cursor->getColors())
		{
			cursor->render(projection, camera->GetRotViewMatrix(), 0.01f);
		} else {
			cursor->render(projection, glm::lookAt({1.0f,0.0f,0.0f},glm::vec3(0.0f),{0.0f,0.0f,1.0f}), 0.01f);
		}
		origin->render(projection, view, 0.5f);

		std::vector<std::string> KeysToDestory;

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
		if (ImGui::Button("Demo Menu")) flags.showDemoMenu = !flags.showDemoMenu;
		ImGui::SameLine();
		if (ImGui::Button("New")) flags.showObjectManager = !flags.showObjectManager;
		ImGui::SameLine();
		if (ImGui::Button("Quit")) flags.doLoop = false;

		bool tempOriginColorState = origin->getColors();
		if (ImGui::Checkbox("Origin", &tempOriginColorState)) origin->setColors(tempOriginColorState);
		ImGui::SameLine();
		bool tempCursorColorState = cursor->getColors();
		if (ImGui::Checkbox("Cursor", &tempCursorColorState)) cursor->setColors(tempCursorColorState);
		ImGui::SameLine();

		bool vsync_check = (SDL_GL_GetSwapInterval() != 0);
		if (ImGui::Checkbox("VSync",&vsync_check))
		{
			if (!vsync_check)
			{
				SDL_GL_SetSwapInterval(0);
			}
			else
			{
				if (SDL_GL_SetSwapInterval(-1) < 0)
					SDL_GL_SetSwapInterval(1);
			}
		}

		//ImGui::ColorEdit3("Background Color", (float*)&clear_color); // Edit 3 floats representing a color
		
		ImGui::DragFloat("Font Size", &fontSize, 0.01f);
		ImGui::SetWindowFontScale(fontSize);
		//ImGui::GetIO().Fonts->
		//font->

		world->Draw(deltaTime, camera);

		ImGui::End();

		ImGui::Begin("Overlay",(bool*)__null,	ImGuiWindowFlags_NoDecoration |
												ImGuiWindowFlags_NoBackground |
												ImGuiWindowFlags_NoInputs |
												ImGuiWindowFlags_NoNav );
		ImGui::SetWindowPos({0,0});

		ImGui::SetWindowFontScale(fontSize);

		ImGui::SetWindowSize( ImGui::GetIO().DisplaySize );

		*fps_array_it = 1000.0f/deltaTime;
		++fps_array_it;
		if (fps_array_it == fps_array.end())
			fps_array_it = fps_array.begin();

		if (fps_count > int(current_fps / 4))
		{
			current_fps = std::accumulate(fps_array.begin(),fps_array.end(),1.0f) / fps_array.size();
			fps_count = 0;
		}
		else ++fps_count;

		ImGui::Text("FPS: %.3f", current_fps);
		ImGui::Text("Camera: %.3f,%.3f,%.3f", camera->Position.x, camera->Position.y, camera->Position.z);
		ImGui::Text("Camera Direction: %.3f,%.3f,%.3f", camera->Front.x, camera->Front.y, camera->Front.z);
		ImGui::Text("Yaw: %.3f, Pitch: %.3f", camera->Yaw, camera->Pitch);
		ImGui::Text("Zoom: %.3f", camera->Zoom);

		for (auto &cube : world->renderables)
			if (cube.first->flags.isHovered)
			{
				ImGui::Text("Looking at: %p", cube.first.get());
				ImGui::Text("Is Selected: %s", cube.first->flags.isSelected ? "true" : "false" );
			}

		ImGui::End();

		//if ( SDL_GetRelativeMouseMode() == SDL_FALSE )
		//{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//}

		// Swap the internal framebuffer to the screen
		SDL_GL_SwapWindow(window);

		std::vector<std::shared_ptr<Renderable> > toD;
		for (auto node = world->renderables.begin(); node != world->renderables.end(); ++node)
			if (node->first->flags.QueueDestruction)
			{
				toD.push_back(node->first);
			}

		for (auto &node : toD)
		{
			auto pnode = world->FindSiblingVectorOfChild(node);

			pnode.first->erase(pnode.second);

			auto &obj = world->renderables[node];
			if (obj.size() > 0)
				pnode.first->insert(pnode.first->end(),obj.begin(),obj.end());

			world->renderables.erase(node);
		}

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
					//for (auto &cube : world->renderables)
						//if ( cube.second->flags.isClosest )
							world->selectClosest = true;
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
		bool showObjectManager;
	} flags;

	SDL_Window *window;
	SDL_GLContext glcontext;

	float fontSize;

	ImColor clear_color;

	std::chrono::steady_clock::time_point start;

	std::shared_ptr<ImFont> font;
	std::array<float, 60UL> fps_array;
	std::array<float, 60UL>::iterator fps_array_it;
	float current_fps;
	int fps_count;

	std::shared_ptr<GLScene> world;
	std::unique_ptr<Origin> origin;
	std::unique_ptr<Origin> cursor;
	std::shared_ptr<Camera> camera;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
