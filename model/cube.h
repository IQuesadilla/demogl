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

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "assets/rawcube.h"

#include "model/model.h"


class myCube : public Model
{
public:
    myCube();
    myCube(myCube *temp);
    ~myCube();

};