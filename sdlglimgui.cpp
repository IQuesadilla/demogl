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

#include "assets/rawcube.h"

#define FullOnStart false
#define myFFlag SDL_WINDOW_FULLSCREEN_DESKTOP
#define AA_LEVEL 2
#define WWIDTH 640
#define WHEIGHT 480

bool RaycastRotatedCube(const glm::vec3& cubeExtents, const glm::mat4& cubeTransform, const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
    // Compute the inverse transformation matrix for the cube
    glm::mat4 inverseTransform = glm::inverse(cubeTransform);

    // Transform the ray into the coordinate space of the cube
    glm::vec3 localRayOrigin = glm::vec3(inverseTransform * glm::vec4(rayOrigin, 1.0));
    glm::vec3 localRayDir = glm::normalize(glm::vec3(inverseTransform * glm::vec4(rayDir, 0.0)));

    // Compute the inverse of the cube's extents
    glm::vec3 inverseExtents = glm::vec3(1.0) / cubeExtents;

    // Compute the minimum and maximum intersection times for the x-axis
    float txMin = (inverseExtents.x * (-cubeExtents.x - localRayOrigin.x)) / localRayDir.x;
    float txMax = (inverseExtents.x * ( cubeExtents.x - localRayOrigin.x)) / localRayDir.x;

    // Compute the minimum and maximum intersection times for the y-axis
    float tyMin = (inverseExtents.y * (-cubeExtents.y - localRayOrigin.y)) / localRayDir.y;
    float tyMax = (inverseExtents.y * ( cubeExtents.y - localRayOrigin.y)) / localRayDir.y;

    // Compute the minimum and maximum intersection times for the z-axis
    float tzMin = (inverseExtents.z * (-cubeExtents.z - localRayOrigin.z)) / localRayDir.z;
    float tzMax = (inverseExtents.z * ( cubeExtents.z - localRayOrigin.z)) / localRayDir.z;

    // Compute the maximum and minimum intersection times for the whole cube
    float tMin = glm::max(glm::max(glm::min(txMin, txMax), glm::min(tyMin, tyMax)), glm::min(tzMin, tzMax));
    float tMax = glm::min(glm::min(glm::max(txMin, txMax), glm::max(tyMin, tyMax)), glm::max(tzMin, tzMax));

	//std::cout << "tMin: " << tMin << ", tMax: " << tMax << std::endl;

    // Check if the ray intersects the cube
    if (tMin > tMax || tMax < 0.0f)
    {
        // The ray missed the cube
        return false;
    }

    // The ray intersects the cube
    return true;
}

class myCube
{
public:
	myCube()
	{
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

		rotAxis = glm::vec3(0.f);
		spinAxis = glm::vec3(0.f);
		alpha = 1.0f;
		trans = glm::vec3(0.f);

		flags.isHovered = false;
		flags.isSelected = false;
		flags.isClosest = false;

		shader = new _shader();
		// Load and compile the basic demo shaders, returns true if error
		if ( shader->load("assets/demo.vert","assets/demo.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << shader->getErrors() << std::endl;
			return;
		}
	}

	myCube(myCube *temp)
	{
		memcpy(this,temp,sizeof(myCube));
	}

	~myCube()
	{
		delete shader;
		glDeleteBuffers(1,&colorbuff);
		glDeleteBuffers(1,&vertbuff);
		glDeleteVertexArrays(1,&VAO);
	}

	void render(glm::mat4 projection, glm::mat4 view, float deltaTime, std::shared_ptr<Camera> cam)
	{
		//if ( !(speed > -0.01f && speed < 0.01f) )
		rotAxis += deltaTime*spinAxis;

		if (rotAxis.x > 360.f)
			rotAxis.x -= 360.f;
		if (rotAxis.y > 360.f)
			rotAxis.y -= 360.f;
		if (rotAxis.z > 360.f)
			rotAxis.z -= 360.f;

		if (rotAxis.x < 0.f)
			rotAxis.x += 360.f;
		if (rotAxis.y < 0.f)
			rotAxis.y += 360.f;
		if (rotAxis.z < 0.f)
			rotAxis.z += 360.f;

		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, trans);

		model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
			glm::angleAxis(							// Angle axis has two arguments - angle and axis
				glm::radians(rotAxis.x),					// The angle to rotate all the vertices, converts deg to rad
				glm::vec3(1.f,0.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
		model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
			glm::angleAxis(							// Angle axis has two arguments - angle and axis
				glm::radians(rotAxis.y),					// The angle to rotate all the vertices, converts deg to rad
				glm::vec3(0.f,1.f,0.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)
		model = model * glm::toMat4(				// Angle axis returns a quaternion - convert into a 4x4 matrix
			glm::angleAxis(							// Angle axis has two arguments - angle and axis
				glm::radians(rotAxis.z),					// The angle to rotate all the vertices, converts deg to rad
				glm::vec3(0.f,0.f,1.f) ));							// Which axis to apply the rotation to and how much - (x,y,z)

		//glm::vec3 lookingDirection = glm::vec3(cam->Front)
		flags.isHovered = RaycastRotatedCube(glm::vec3(1.0f, 1.0f, 1.0f), model, cam->Position, cam->Front);

		glBindVertexArray(VAO);

		if ( !flags.isSelected )	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		else						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		//if ( flags.isHovered )
		//	alpha = 0.3f;
		//else
		//	alpha = 1.0f;

		// Use shader "shader" and give it all 3 uniforms
		shader->use();
		shader->setMat4("model",model);				// GLSL: uniform mat4 model;
		shader->setMat4("view",view);				// GLSL: uniform mat4 view;
		shader->setMat4("projection",projection);	// GLSL: uniform mat4 projection;

		if (flags.isClosest)
			shader->setFloat("alpha",alpha/2);
		else shader->setFloat("alpha",alpha);

		flags.isClosest = false;

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
	}

	float distance(glm::vec3 pos)
	{
		return glm::abs(glm::distance(pos,trans));
	}

	struct {
		bool isHovered;
		bool isSelected;
		bool isClosest;
	} flags;

	GLuint VAO, vertbuff, colorbuff;
	float alpha;
	glm::vec3 trans, rotAxis, spinAxis, spin;
	_shader *shader;
	glm::vec3 hitPoint;
};

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

		origin.reset( new _shader() );
		if ( origin->load("assets/origin.vert","assets/origin.frag") )
		{
			std::cout << "Failed to load shaders!" << std::endl << origin->getErrors() << std::endl;
			return;
		}

		glGenVertexArrays(1,&originVAO);
		glBindVertexArray(originVAO);

		// Generate a Vertex Buffer Object to represent the cube's vertices
		glGenBuffers(1,&originvertbuff);
		glBindBuffer(GL_ARRAY_BUFFER, originvertbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(originVerts), originVerts, GL_STATIC_DRAW);

		glGenBuffers(1,&origincolorbuff);
		glBindBuffer(GL_ARRAY_BUFFER, origincolorbuff);
		glBufferData(GL_ARRAY_BUFFER, sizeof(originColors), originColors, GL_STATIC_DRAW);

		glLineWidth(2.0f);

		// Enable depth test - makes things in front appear in front
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

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

		std::list<std::pair<std::shared_ptr<myCube>, float> > sorted;
		std::pair<std::shared_ptr<myCube>, float> closestHovered
			= std::make_pair(nullptr, 1000.0f);

		for (auto &cube : cubes)
		{
			if (cube->flags.isHovered)
			{
				float dis = cube->distance(camera->Position);
				if ( dis < closestHovered.second )
				{
					closestHovered = std::make_pair(cube,dis);
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

		for (auto &cube : cubes)
		{
			if (cube->flags.isSelected) glEnable(GL_CULL_FACE);
			else						glDisable(GL_CULL_FACE);

			if (cube->alpha == 1.0f && !cube->flags.isClosest)
				cube->render(projection,view,deltaTime, camera);
			else
			{
				bool isInserted = false;
				float dis = cube->distance(camera->Position);
				for (auto it = sorted.begin(); it != sorted.end(); it++)
				{
					if ( dis > it->second )
					{
						sorted.insert(it,std::make_pair(cube,dis));
						isInserted = true;
						break;
					}
				}

				if ( !isInserted )
				{
					sorted.emplace_back(std::make_pair(cube,dis));
				}
			}
		}

		glBindVertexArray(originVAO);

		origin->use();
		origin->setMat4("projection",projection);
		origin->setMat4("view",view);

		glBindBuffer(GL_ARRAY_BUFFER, originvertbuff);
		glVertexAttribPointer(
			0,                  // location
			3,                  // size (per vertex)
			GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
			GL_FALSE,           // is normalized*
			0,                  // stride**
			(void*)0            // array buffer offset
		);

		glBindBuffer(GL_ARRAY_BUFFER, origincolorbuff);
		glVertexAttribPointer(
			1,                  // location
			3,                  // size (per vertex)
			GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
			GL_FALSE,           // is normalized*
			0,                  // stride**
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		// Draw the cube
		glDrawArrays(GL_LINES, 0, 6);

		// Disable location 0 and location 1
		glDisableVertexArrayAttrib(originVAO, 0);
		glDisableVertexArrayAttrib(originVAO, 1);



		//glDepthMask( GL_FALSE );
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (auto &cube : sorted)
		{
			cube.first->render(projection,view,deltaTime,camera);
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

			ImGui::Text("Cubes: %ld, Sorted: %ld", cubes.size(), sorted.size());

			int i = 0;
			for (auto &cube : cubes)
			{
				std::string name = std::to_string(i++);
				if ( cube->flags.isSelected && ImGui::TreeNode(name.c_str()) )
				{
					ImGui::SliderFloat("Alpha", &cube->alpha, 0.0f, 1.0f);

					ImGui::DragFloat("Spin X", &cube->spinAxis.x, 0.01f);
					ImGui::DragFloat("Spin Y", &cube->spinAxis.y, 0.01f);
					ImGui::DragFloat("Spin Z", &cube->spinAxis.z, 0.01f);

					ImGui::DragFloat("Rot X", &cube->rotAxis.x);
					ImGui::DragFloat("Rot Y", &cube->rotAxis.y);
					ImGui::DragFloat("Rot Z", &cube->rotAxis.z);

					ImGui::DragFloat("X", &cube->trans.x, 0.01f);
					ImGui::DragFloat("Y", &cube->trans.y, 0.01f);
					ImGui::DragFloat("Z", &cube->trans.z, 0.01f);

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

		ImGui::SetWindowSize( ImGui::GetIO().DisplaySize );

		ImGui::Text("FPS: %.3f", 1000.0f/deltaTime);
		ImGui::Text("Camera: %.3f,%.3f,%.3f", camera->Position.x, camera->Position.y, camera->Position.z);
		ImGui::Text("Camera Direction: %.3f,%.3f,%.3f", camera->Front.x, camera->Front.y, camera->Front.z);
		ImGui::Text("Yaw: %.3f, Pitch: %.3f", camera->Yaw, camera->Pitch);
		ImGui::Text("Zoom: %.3f", camera->Zoom);

		std::string crosshair = "   |   \n---+---\n   |   ";
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

		if ( addCube ) cubes.push_back( std::make_shared<myCube>( new myCube() ) );
		if ( clearCubes ) cubes.clear();

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
					for (auto &cube : cubes)
						if ( cube->flags.isHovered )
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

	ImColor clear_color;

	std::chrono::_V2::steady_clock::time_point start;

	std::vector<std::shared_ptr<myCube> > cubes;

	std::shared_ptr<Camera> camera;

	std::unique_ptr<_shader> origin;
	GLuint originVAO, originvertbuff, origincolorbuff;
};

int main()
{
	gldemo obj;
	while (obj.isAlive()) obj.runOnce();
	return 0;
}
