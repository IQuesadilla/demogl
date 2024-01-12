#define SDL_MAIN_HANDLED
#include "glad/glad.h"
#include "glad/khrplatform.h"
#include "SDL.h"
#if defined __has_include
    #if __has_include (<SDL_opengl.h>)
        #include <SDL_opengl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif
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
#include "scene/loader.h"
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

		SDL_SetMainReady();

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

		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			printf( "Failed loading glad" );
			return;
		}
    else std::cout << "Successfully loaded GLAD" << std::endl;

		#if AA_LEVEL
			glEnable(GL_MULTISAMPLE);
		#endif

    cv::setNumThreads(1);

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

    SceneLoader.reset(new GLSceneLoader());

		world.reset( new GLScene() );
    SceneLibrary["RootScene"] = world;
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

		//for (auto &x : fps_array)
		//	x = 1.0f;
		//fps_array_it = fps_array.begin();
    previous_fps = 1.0f;

		if (SDL_GL_SetSwapInterval(-1) < 0)
			SDL_GL_SetSwapInterval(1);
		SDL_ShowWindow(window);

		std::cout << "Setup Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << "mS" << std::endl;
		start = std::chrono::steady_clock::now();
    fps_start = std::chrono::steady_clock::now();

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
		if ( SDL_GetRelativeMouseMode() == SDL_FALSE ) ImGui_ImplSDL2_NewFrame();
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
    camera->BuildProjectionMatrix(0.5f,86.7f); // Needs to run every frame
		glm::mat4 projection = camera->ProjectionMatrix;
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
    std::shared_ptr<GLScene> SelectedWorld = nullptr;

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
		    if (ImGui::MenuItem("Fullscreen"))
		    {
		    	// Check if my fullscreen flag is set, set it to opposite
		    	bool isF = SDL_GetWindowFlags(window) & myFFlag;
		    	SDL_SetWindowFullscreen(window, isF ? 0 : myFFlag);
		    }
		    if (ImGui::MenuItem("Demo Menu")) flags.showDemoMenu = !flags.showDemoMenu;
		    if (ImGui::MenuItem("Import File"))
        {
          //flags.showObjectManager = true;
          myFileBrowser.SetTitle("Import");
          myFileBrowser.SetPwd("assets/");
          myFileBrowser.Open();
          //myFileBrowser.Display();
        }
		    if (ImGui::MenuItem("Quit")) flags.doLoop = false;
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Window"))
      {
		    bool tempOriginColorState = origin->getColors();
		    if (ImGui::MenuItem("Colored Origin",NULL,tempOriginColorState)) origin->setColors(!tempOriginColorState);
		    bool tempCursorColorState = cursor->getColors();
		    if (ImGui::MenuItem("Colored Cursor",NULL,tempCursorColorState)) cursor->setColors(!tempCursorColorState);

        ImGui::DragFloat("Font Size", &fontSize, 0.01f);
		    ImGui::SetWindowFontScale(fontSize);

		    bool vsync_check = (SDL_GL_GetSwapInterval() != 0);
		    if (ImGui::MenuItem("VSync",NULL,vsync_check))
		    {
          vsync_check = !vsync_check;
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
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Scene"))
      {
        ImGui::Text("%p",world.get());
        //if (ImGui::MenuItem("Temp Load Item")) SceneLoader->QueueFile("thisone");
        //if (ImGui::MenuItem("Scene Select.."))
        //  ImGui::OpenPopup("my_select_popup"); 
        if (ImGui::BeginMenu("Scene Select"))
        {
          //ImGui::SeparatorText((std::string("Available Scenes: ") + std::to_string(LoadedScenes.size())).c_str());
          for (auto &it : SceneLibrary)
            if (ImGui::MenuItem(it.first.c_str(),NULL,world == it.second))
               SelectedWorld = it.second;
          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

		if ( flags.showDemoMenu )
			ImGui::ShowDemoWindow(&flags.showDemoMenu);

    bool isRelMouseMode = SDL_GetRelativeMouseMode() == SDL_TRUE;
    ImGui::SetNextWindowCollapsed(isRelMouseMode,ImGuiCond_Always);
    //ImGui::SetNextWindowPos({0,100});
    //ImGui::SetNextWindowSize(ImVec2(0,400));

    float ScreenHeight = ImGui::GetMainViewport()->Size.y;
		ImGui::Begin("Menu",(bool*)0, ImGuiWindowFlags_NoMove | (isRelMouseMode ? 0 : ImGuiWindowFlags_NoCollapse));
    float MenuWindowHeight = ImGui::GetWindowHeight();
    //float MenuWindowPosY = ImGui::GetWindowPos().y;
    ImGui::SetWindowPos({0,ScreenHeight - MenuWindowHeight});
    //ImGui::SetWindowSize(ImVec2(0,ScreenHeight - MenuWindowPosY));

    ImGui::Text("Press ESCAPE to toggle menu");

		world->Draw(deltaTime, camera);

		ImGui::End();
    myFileBrowser.Display();
    if ( myFileBrowser.HasSelected() )
    {
      SceneLoader->QueueFile(myFileBrowser.GetSelected());
      myFileBrowser.ClearSelected();
    }

		ImGui::Begin("Overlay",(bool*)0,	ImGuiWindowFlags_NoDecoration |
												ImGuiWindowFlags_NoBackground |
												ImGuiWindowFlags_NoInputs |
												ImGuiWindowFlags_NoNav );
		ImGui::SetWindowPos({0,20});

		ImGui::SetWindowFontScale(fontSize);

		ImGui::SetWindowSize( ImGui::GetIO().DisplaySize );

		float InverseDeltaTime = 1000.0f/deltaTime;
		//++fps_array_it;
		//if (fps_array_it == fps_array.end())
		//	fps_array_it = fps_array.begin();
    
    min_fps = glm::min(min_fps, InverseDeltaTime);
    max_fps = glm::max(max_fps, InverseDeltaTime);

    current_fps += InverseDeltaTime;
    ++fps_count;
		if (start > fps_start + std::chrono::milliseconds(500))
		{

		//	current_fps = std::accumulate(fps_array.begin(),fps_array.end(),1.0f) / fps_array.size();
      previous_fps = current_fps / float(fps_count);
      current_fps = 0;
			fps_count = 0;
      previous_min_fps = min_fps;
      previous_max_fps = max_fps;
      min_fps = INFINITY;
      max_fps = 0.f;
      fps_start = std::chrono::steady_clock::now();
		}
		//else ++fps_count;

		ImGui::Text("FPS: %8.3f,%8.3f,%8.3f", previous_fps,previous_min_fps,previous_max_fps);
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

    if (SelectedWorld)
      world = SelectedWorld;

    std::chrono::steady_clock::time_point BeginRetrieve = std::chrono::steady_clock::now();
    auto ImportedScene = SceneLoader->Retrieve();
    if (ImportedScene.second)
    {
      std::cout << "Retrieve took " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
        << " milliseconds" << std::endl;
      BeginRetrieve = std::chrono::steady_clock::now();

      //GLScene *tempptr = new GLScene(); 
      if (SceneLibrary.find(ImportedScene.first) == SceneLibrary.end())
      {
        SceneLibrary[ImportedScene.first] = ImportedScene.second;
        ImportedScene.second->GLInit();

        std::cout << "Scene GLInit() took " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
          << " milliseconds" << std::endl;
        BeginRetrieve = std::chrono::steady_clock::now();

        for (auto &x : ImportedScene.second->models)
        {
          //x.second->shader = ImportedScene.second->AABBShader;
          x.second->GLInit();
        }
        std::cout << "Model GLInit() took " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
          << " milliseconds" << std::endl;
        //tempptr->ImportScene(ImportedScene.second.get());
      }
      else
      {
        std::cout << "Imported Scene already exists!" << std::endl;
      }
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
  ImGui::FileBrowser myFileBrowser;

	float fontSize;

	ImColor clear_color;

	std::chrono::steady_clock::time_point start;

	std::shared_ptr<ImFont> font;
	//std::array<float, 60UL> fps_array;
	//std::array<float, 60UL>::iterator fps_array_it;
	float current_fps;
	int fps_count;
  float previous_fps;
  float previous_min_fps, min_fps, previous_max_fps, max_fps;
  std::chrono::steady_clock::time_point fps_start;

	std::shared_ptr<GLScene> world;
  std::unordered_map<std::string,std::shared_ptr<GLScene>> SceneLibrary;
	std::unique_ptr<Origin> origin;
	std::unique_ptr<Origin> cursor;
	std::shared_ptr<Camera> camera;
  std::shared_ptr<GLSceneLoader> SceneLoader;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
