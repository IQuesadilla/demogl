#pragma once

#include "glad/glad.h"
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