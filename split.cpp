#define SDL_MAIN_HANDLED
#include "glad/glad.h"
//#include "glad/khrplatform.h"
#include "SDL.h"
#include "shader.h"
#include "camera.h"
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include "log.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "scene/imfilebrowser.h"

#include "scene/scene.h"
#include "scene/loader.h"
#include "renderable/renderable.h"
#include "origin/origin.h"
//#include "model/model.h"
#include "model/cube.h"
#include "model/window.h"

#define FullOnStart false
#define myFFlag SDL_WINDOW_FULLSCREEN_DESKTOP
#define AA_LEVEL 8
#define WWIDTH 1280
#define WHEIGHT 720

#include "imgui.h"
#include "colorcodes.h"
#include <string>
#include <unordered_map>
#include <regex>

// Function to map escape codes to ImGui colors
ImVec4 GetColorFromEscapeCode(const std::string& code) {
  static const std::unordered_map<std::string, ImVec4> colorMap = {
    { RESET, ImGui::GetStyleColorVec4(ImGuiCol_Text) },     // RESET - White (assuming default)
    { BLACK, ImVec4(0.0f, 0.0f, 0.0f, 1.0f) },    // BLACK
    { RED, ImVec4(1.0f, 0.0f, 0.0f, 1.0f) },    // RED
    { GREEN, ImVec4(0.0f, 1.0f, 0.0f, 1.0f) },    // GREEN
    { YELLOW, ImVec4(1.0f, 1.0f, 0.0f, 1.0f) },    // YELLOW
    { BLUE, ImVec4(0.0f, 0.0f, 1.0f, 1.0f) },    // BLUE
    { MAGENTA, ImVec4(1.0f, 0.0f, 1.0f, 1.0f) },    // MAGENTA
    { CYAN, ImVec4(0.0f, 1.0f, 1.0f, 1.0f) },    // CYAN
    { WHITE, ImVec4(1.0f, 1.0f, 1.0f, 1.0f) },    // WHITE
  };

  auto it = colorMap.find(code);
  if (it != colorMap.end()) {
    return it->second;
  }

  return ImGui::GetStyleColorVec4(ImGuiCol_Text); // Default color
}

// Function to render colored text
void RenderAnsiColoredText(const std::string& text) {
  static const std::regex colorRegex("\033\\[[0-9;]*m");
  std::sregex_iterator wordsBegin = std::sregex_iterator(text.begin(), text.end(), colorRegex);
  std::sregex_iterator wordsEnd = std::sregex_iterator();

  size_t lastPos = 0;
  for (std::sregex_iterator i = wordsBegin; i != wordsEnd; ++i)
  {
    std::smatch match = *i;
    size_t matchPos = match.position();

    // Render text before escape code
    if (matchPos > lastPos)
    {
      //std::cout << text.substr(lastPos, matchPos - lastPos) << std::endl;
      ImGui::TextUnformatted(text.substr(lastPos, matchPos - lastPos).c_str());
      ImGui::SameLine();

      if (lastPos != 0) ImGui::PopStyleColor();
      ImVec4 color = GetColorFromEscapeCode(match.str());
      ImGui::PushStyleColor(ImGuiCol_Text, color);
      //ImGui::SameLine(0.0f, 0.0f);
    }

    //if (i != wordsBegin) ImGui::PopStyleColor();

    // Apply color 

    lastPos = matchPos + match.length();
  }

  // Render remaining text
  if (lastPos < text.length()) {
    ImGui::TextUnformatted(text.substr(lastPos).c_str());
  } else ImGui::TextUnformatted("");
  if (lastPos != 0) ImGui::PopStyleColor();

  //ImGui::PopStyleColor(); // Pop the last color
}

struct ExampleAppLog
{
  ImGuiTextBuffer     Buf;
  ImGuiTextFilter     Filter;
  ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
  bool                AutoScroll;  // Keep scrolling if already at the bottom.

  ExampleAppLog()
  {
    AutoScroll = true;
    Clear();
  }

  void    Clear()
  {
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
  }

  void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
  {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n')
        LineOffsets.push_back(old_size + 1);
  }

  void    Draw(const char* title, bool* p_open = NULL)
  {
    if (!ImGui::Begin(title, p_open))
    {
      ImGui::End();
      return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
      ImGui::Checkbox("Auto-scroll", &AutoScroll);
      ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
      ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
      if (clear)
        Clear();
      if (copy)
        ImGui::LogToClipboard();

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      const char* buf = Buf.begin();
      const char* buf_end = Buf.end();
      if (Filter.IsActive())
      {
        // In this example we don't use the clipper when Filter is enabled.
        // This is because we don't have random access to the result of our filter.
        // A real application processing logs with ten of thousands of entries may want to store the result of
        // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
        for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
        {
          const char* line_start = buf + LineOffsets[line_no];
          const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
          if (Filter.PassFilter(line_start, line_end))
            RenderAnsiColoredText(std::string(line_start, line_end));
        }
      }
      else
      {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
        ImGuiListClipper clipper;
        clipper.Begin(LineOffsets.Size);
        while (clipper.Step())
        {
          for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
          {
            const char* line_start = buf + LineOffsets[line_no];
            const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
            RenderAnsiColoredText(std::string(line_start, line_end));
          }
        }
        clipper.End();
      }
      ImGui::PopStyleVar();

      // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
      // Using a scrollbar or mouse-wheel will take away from the bottom edge.
      if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::End();
  }
};

class gldemo
{
public:
	gldemo()
	{
		// Don't enable the main loop unless init succeeds
		flags.doLoop = false;

		SDL_SetMainReady();

    logobj = libQ::log(libQ::DEFAULT);

		start = std::chrono::steady_clock::now();
    logobj.setClass("gldemo");
    logobj[1].setVerbosity(libQ::DEBUG);
    logobj[1].setCallback(*this,&gldemo::PrintCallback);

    LogFile.open("split.log");
    if (LogFile)
    {
      logobj[2].setColors(false);
      logobj[2].setVerbosity(libQ::DEBUG);
      logobj[2].setCallback(*this,&gldemo::LogFileCallback);
    } else std::cerr << "Failed to open log file" << std::endl;

    auto log = logobj("gldemo");

		// Initialize Video and Events on SDL, no need to initialize any other subsystems
		if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) < 0 )
		{
			std::cerr << "Failed to init SDL! SDL: " << SDL_GetError() << std::endl;
			return;
		}

    log << "Video Drivers Available:";
    int DriverCount = SDL_GetNumVideoDrivers();
    for (int i = 0; i < DriverCount; ++i) {
        log << " " << SDL_GetVideoDriver(i);
    }
    log << libQ::NOTEV;
    log << "Current Video Driver: " << SDL_GetCurrentVideoDriver() << libQ::NOTEV;

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

		window.Attach( SDL_CreateWindow(	"gldemo", 					// Window Title
									SDL_WINDOWPOS_UNDEFINED,	// Starting Global X Position
									SDL_WINDOWPOS_UNDEFINED,	// Starting Global Y Position
									WWIDTH,						// Window Width
									WHEIGHT,					// Window Height
									SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_RESIZABLE ) );

		// window will be NULL if CreateWindow failed
		if( window == NULL )
    {
      std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
      return;
    }
    //window.Attach(twindow);

		// start OpenGL within the SDL window, returns NULL if failed
		glcontext.Attach (SDL_GL_CreateContext( window ));
		if( glcontext == NULL )
		{
      std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
			return;
		}

    SDL_GL_MakeCurrent(window, glcontext);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
      std::cerr << "Failed loading glad" << std::endl;
			return;
		}
    //else std:cout << "Successfully loaded GLAD" << std::endl;

    // Should not cout after this point
    //auto log = logobj->function("split");

		#if AA_LEVEL
      log << "Attempting to enable AntiAliasing" << libQ::NOTEDEBUG;
			glEnable(GL_MULTISAMPLE);
		#endif

    cv::setNumThreads(1);

		// Load window icon and set if successfully loaded
		SDL_Surface *icon = SDL_LoadBMP("assets/opengl.bmp");
		if (icon == NULL)
		{
			log << "Failed to load icon image" << libQ::WARNING;
		}
		else
		{
			SDL_SetWindowIcon(window, icon);
			SDL_FreeSurface(icon);
		}

    log << "Creating camera object" << libQ::NOTEDEBUG;
		// Create a new camera and set it's default position to (x,y,z)
		// +z is towards you, -z is away from you

    log << "Enabling OpenGL Depth Test" << libQ::NOTEDEBUG;
		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

    log << "Starting the SceneLoader Thread" << libQ::NOTEDEBUG;
    SceneLoader.reset(new GLSceneLoader(logobj));

    log << "Building initial empty scene" << libQ::NOTEDEBUG;
		world.reset( new GLScene(logobj) );

    log << "Generating Default World Objects" << libQ::NOTEDEBUG;
    DefaultSkyboxVAO.Generate();
    DefaultAABBVAO.Generate();
    DefaultSkyboxTex.Generate();
      
    //auto image = cv::imread("assets/skybox.png");

    DefaultShader.reset(new _shader("assets/solid_color.vert","assets/solid_color.frag"));
    DefaultSkyboxShader.reset(new _shader("assets/skybox.vert","assets/skybox.frag"));
    world->ImportWorldOptions(
        DefaultSkyboxVAO, DefaultAABBVAO,
        DefaultSkyboxTex,
        DefaultSkyboxShader, DefaultShader);
    world->GLInit();
    world->UpdateSkybox(cv::imread("assets/skybox.png"));
    //DefaultSkyboxTex = world->SkyboxTexID;
    //DefaultShader->load("assets/solid_color.vert","assets/solid_color.frag");
    //SkyboxShader->load("assets/skybox.vert","assets/skybox.frag");

    SceneLibrary["RootScene"] = world;
		world->shaders["basic_textured"].reset( new _shader("assets/basic_textured.vert","assets/basic_textured.frag") );
		world->models["blank"].reset( new Model() );
		world->models["cube"].reset( new myCube() );
		world->models["cube"]->shader = world->shaders["basic_textured"];
		world->models["window"].reset( new Window() );
		world->models["window"]->shader = world->shaders["basic_textured"];

		/*for (auto &model : world->models)
			if (model.second->shader == nullptr)
			{
				std:cout << "Set default shader for \"" << model.first << '"' << std::endl;
				model.second->shader.reset(&world->DefaultShader);
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
		ImFontConfig font_cfg = {};
		font_cfg.OversampleH = 2;
		font_cfg.OversampleV = 2;
		//font_cfg.RasterizerFlags |= ImGuiFreeType::ForceAutoHint;
		font.reset(ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/font.ttf",15,&font_cfg));
		flags.showDemoMenu = false;
		flags.showObjectManager = false;
    flags.isShowingLog = false;

		#if FullOnStart
			SDL_SetWindowFullscreen(window, myFFlag);
		#endif

		GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
      std::cerr << "INIT Error:" << err << std::endl;
    }

		GLfloat range[2] = {0.0f,0.0f};
		glGetFloatv(GL_LINE_WIDTH_RANGE, range);
		log << "Line Width: " << range[0] << ", " << range[1] << libQ::VALUEVV;
		glGetFloatv(GL_POINT_SIZE_RANGE, range);
		log << "Point Size: " << range[0] << ", " << range[1] << libQ::VALUEVV;

		int depthBufferSize;
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthBufferSize);
		log << "Allocated Depth Buffer Size: " << depthBufferSize << libQ::VALUEVV;

		fontSize = 1.0f;
    FOV = 1.0f;

		//for (auto &x : fps_array)
		//	x = 1.0f;
		//fps_array_it = fps_array.begin();
    previous_fps = 1.0f;

		if (SDL_GL_SetSwapInterval(-1) < 0)
			SDL_GL_SetSwapInterval(1);
		SDL_ShowWindow(window);

		log << "Setup Time: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count()
        << "ms" << libQ::VALUE;
		start = std::chrono::steady_clock::now();
    fps_start = std::chrono::steady_clock::now();

		// If here, initialization succeeded and loop should be enabled
		flags.doLoop = true;
	}

	~gldemo()
	{
    auto log = logobj("gldemo::~gldemo");
		// Properly shutdown OpenGL and destroy the window
		//SDL_GL_DeleteContext(glcontext);
		//SDL_DestroyWindow(window);
	}

	void runOnce()
	{
    auto log = logobj("runOnce",libQ::DELAYPRINTFUNCTION);
    //log << "Testingggg" << libQ::NOTE;
		ImGui_ImplOpenGL3_NewFrame();
		if ( SDL_GetRelativeMouseMode() == SDL_FALSE ) ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// Check for any inputs from the user
		pollInput();

		// Get amount of time since last frame in milliseconds, used to determine how much movement is necessary
		float deltaTime = float((std::chrono::steady_clock::now() - start).count()) / 1000000.0f;
		start = std::chrono::steady_clock::now();

		// Calculate new matrices given the time since the last frame
		world->camera->InputUpdate(deltaTime);

		// Projection and view are the same per model because they are affected by the camera
		//glMatrixMode(GL_PROJECTION);
    world->camera->BuildProjectionMatrix(AspectRatio*FOV,0.5f,870.f); // Needs to run every frame
		glm::mat4 projection = world->camera->ProjectionMatrix;
		glm::mat4 view = world->camera->GetViewMatrix();

		glDepthMask( GL_TRUE );

		// Set clear color, clear screen and depth
		glClearColor(clear_color.Value.x,clear_color.Value.y,clear_color.Value.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (cursor->getColors())
		{
			cursor->render(projection, world->camera->GetRotViewMatrix(), 0.01f);
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
        if (ImGui::MenuItem("Show Log", nullptr, flags.isShowingLog)) flags.isShowingLog = !flags.isShowingLog;
		    if (ImGui::MenuItem("Quit")) flags.doLoop = false;
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Window"))
      {
		    bool tempOriginColorState = origin->getColors();
		    if (ImGui::MenuItem("Colored Origin",NULL,tempOriginColorState)) origin->setColors(!tempOriginColorState);
		    bool tempCursorColorState = cursor->getColors();
		    if (ImGui::MenuItem("Colored Cursor",NULL,tempCursorColorState)) cursor->setColors(!tempCursorColorState);

        // TODO: Resizeable fonts
        //if (ImGui::DragFloat("Font Size", &fontSize, 0.01f, 0.1f, 5.f)) {
        //  ImGui::GetStyle().ScaleAllSizes(fontSize / oldsize);
        //  }
		    //ImGui::SetWindowFontScale(fontSize);

        ImGui::DragFloat("FOV",&FOV,0.01,0.1f,10.0f);

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

        if (ImGui::BeginMenu("Scene Library"))
        {
          static std::string ToImport = "<none>";
          if (ImGui::Button("Select...")) ImGui::OpenPopup("SceneImportPopup");
          ImGui::SameLine();
          ImGui::Text("%s",ToImport.c_str());

          if (ImGui::BeginPopup("SceneImportPopup"))
          {
            for (auto &x : SceneLibrary)
              if (x.second != world && ImGui::Selectable(x.first.c_str())) ToImport = x.first;
            ImGui::EndPopup();
          }

          bool Disable = false;
          if (SceneLibrary.find(ToImport) == SceneLibrary.end() || SceneLibrary[ToImport] == world) Disable = true;
          
          if (ImGui::MenuItem("New Empty"))
          {
            std::string NewEmptyName = "Empty0";
            for (int i = 0;
              SceneLibrary.find(NewEmptyName) != SceneLibrary.end();
              NewEmptyName = "Empty" + std::to_string(++i));
            //std::string NewEmptyName = "Empty" + std::to_string(i);
            std::shared_ptr<GLScene> NewScene;
            NewScene.reset(new GLScene(logobj));
            SceneLibrary.emplace(NewEmptyName,NewScene);
            NewScene->ImportWorldOptions(
              DefaultSkyboxVAO, DefaultAABBVAO,
              DefaultSkyboxTex,
              DefaultSkyboxShader, DefaultShader);
            NewScene->GLInit();
          }
          if (Disable) ImGui::BeginDisabled();
          if (ImGui::MenuItem("Import Models")) world->ImportModelsFrom(SceneLibrary[ToImport].get());
          if (ImGui::MenuItem("Import All")) world->ImportScene(SceneLibrary[ToImport].get());
          if (ImGui::MenuItem("Import All & Erase"))
            { world->ImportScene(SceneLibrary[ToImport].get()); SceneLibrary.erase(ToImport); }
          if (ImGui::MenuItem("Erase")) SceneLibrary.erase(ToImport);
          if (Disable) ImGui::EndDisabled();
          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

		if ( flags.showDemoMenu )
			ImGui::ShowDemoWindow(&flags.showDemoMenu);

    if ( flags.isShowingLog )
      VisualLog.Draw("Program Log",&flags.isShowingLog);

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

		world->Draw(deltaTime);

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

		//ImGui::SetWindowFontScale(fontSize);

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
		ImGui::Text("Camera: %.3f,%.3f,%.3f", world->camera->Position.x, world->camera->Position.y, world->camera->Position.z);
		ImGui::Text("Camera Direction: %.3f,%.3f,%.3f", world->camera->Front.x, world->camera->Front.y, world->camera->Front.z);
    ImGui::Text("Aspect Ratio: %.3f", AspectRatio);
		ImGui::Text("Yaw: %.3f, Pitch: %.3f", world->camera->Yaw, world->camera->Pitch);
		ImGui::Text("Zoom: %.3f", world->camera->Zoom);

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
      log << "Retrieve took " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
        << " milliseconds" << libQ::VALUEV;
      BeginRetrieve = std::chrono::steady_clock::now();

      //GLScene *tempptr = new GLScene(); 
      if (SceneLibrary.find(ImportedScene.first) == SceneLibrary.end())
      {
        SceneLibrary[ImportedScene.first] = ImportedScene.second;
        ImportedScene.second->ImportWorldOptions(
          DefaultSkyboxVAO, DefaultAABBVAO,
          DefaultSkyboxTex,
          DefaultSkyboxShader,DefaultShader);
        ImportedScene.second->GLInit();

        log << "Scene GLInit() took " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
          << " milliseconds" << libQ::VALUEV;
        BeginRetrieve = std::chrono::steady_clock::now();

        /*for (auto &x : ImportedScene.second->models)
        {
          //x.second->shader = ImportedScene.second->DefaultShader;
          x.second->GLInit();
        }
        std:cout << "Model GLInit() took " <<
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - BeginRetrieve).count()
          << " milliseconds" << std::endl;*/
        //tempptr->ImportScene(ImportedScene.second.get());
      }
      else
      {
        log << "Imported Scene already exists!" << libQ::ERROR;
      }
    }

		return;
	}

	bool isAlive()
	{
		// Continue to loop while the flag is set to true
		return flags.doLoop;
	}

  void PrintCallback(const std::string output)
  {
    //std::cout << output << std::flush;
    VisualLog.AddLog("%s",output.c_str());
  }

  void LogFileCallback(const std::string output)
  {
    if (LogFile) LogFile << output << std::flush;
  }

private:
	void pollInput()
	{
    auto log = logobj("pollInput",libQ::DELAYPRINTFUNCTION);
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
						world->camera->ProcessKeyboard(FORWARD,true);
					break;
					case SDLK_s:
						world->camera->ProcessKeyboard(BACKWARD,true);
					break;
					case SDLK_d:
						world->camera->ProcessKeyboard(RIGHT,true);
					break;
					case SDLK_a:
						world->camera->ProcessKeyboard(LEFT,true);
					break;
					case SDLK_SPACE:
						world->camera->ProcessKeyboard(UP,true);
					break;
					case SDLK_LSHIFT:
						world->camera->ProcessKeyboard(DOWN,true);
					break;
					case SDLK_UP:
						world->camera->ProcessKeyboard(V_UP,true);
					break;
					case SDLK_DOWN:
						world->camera->ProcessKeyboard(V_DOWN,true);
					break;
					case SDLK_RIGHT:
						world->camera->ProcessKeyboard(V_RIGHT,true);
					break;
					case SDLK_LEFT:
						world->camera->ProcessKeyboard(V_LEFT,true);
					break;
					}
				break;

				case SDL_KEYUP:
					switch (e.key.keysym.sym)
					{
					case SDLK_w:
						world->camera->ProcessKeyboard(FORWARD,false);
					break;
					case SDLK_s:
						world->camera->ProcessKeyboard(BACKWARD,false);
					break;
					case SDLK_d:
						world->camera->ProcessKeyboard(RIGHT,false);
					break;
					case SDLK_a:
						world->camera->ProcessKeyboard(LEFT,false);
					break;
					case SDLK_SPACE:
						world->camera->ProcessKeyboard(UP,false);
					break;
					case SDLK_LSHIFT:
						world->camera->ProcessKeyboard(DOWN,false);
					break;
					case SDLK_UP:
						world->camera->ProcessKeyboard(V_UP,false);
					break;
					case SDLK_DOWN:
						world->camera->ProcessKeyboard(V_DOWN,false);
					break;
					case SDLK_RIGHT:
						world->camera->ProcessKeyboard(V_RIGHT,false);
					break;
					case SDLK_LEFT:
						world->camera->ProcessKeyboard(V_LEFT,false);
					break;
					}
				break;

				case SDL_MOUSEMOTION:
					world->camera->ProcessMouseMovement(e.motion.xrel,e.motion.yrel,true);
				break;

				case SDL_MOUSEWHEEL:
					world->camera->ProcessMouseScroll(e.wheel.preciseY);
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
        AspectRatio = Camera::setViewSize(w,h);
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
    bool isShowingLog;
	} flags;

  //glcontext is guaranteed to be destoryed before window
  SharedSDLWindow window;
  SharedSDLGLContext glcontext;

  ImGui::FileBrowser myFileBrowser;
  ExampleAppLog VisualLog;

	float fontSize;
  float AspectRatio;
  float FOV;

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

  std::ofstream LogFile;

	std::shared_ptr<GLScene> world;
  std::unordered_map<std::string,std::shared_ptr<GLScene>> SceneLibrary;
	std::unique_ptr<Origin> origin;
	std::unique_ptr<Origin> cursor;
  std::shared_ptr<_shader> DefaultShader, DefaultSkyboxShader;
  SharedVAO DefaultSkyboxVAO, DefaultAABBVAO;
  SharedTex DefaultSkyboxTex;
  //SharedVBO DefaultSkyboxVBO, DefaultAABBVBO;
  std::shared_ptr<GLSceneLoader> SceneLoader;

  libQ::log logobj;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
