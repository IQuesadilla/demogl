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
#include <chrono>

#include "model/model.h"


class Blank : public Model
{
public:
    Blank();
    Blank(Blank *_new);
};