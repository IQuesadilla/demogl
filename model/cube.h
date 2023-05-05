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

#include "model/model.h"


class myCube : public Model
{
public:
    myCube();
    myCube(myCube *_new);
    ~myCube();

};