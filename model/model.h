#ifndef MODEL_H
#define MODEL_H
#pragma once

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "model/model.h"
#include "shader/shader.h"

#include "SDL.h"
#include <SDL_opengl.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

#include <string>
#include <memory>

class Model
{
public:
    
    Model(std::shared_ptr<_shader> shader);
    Model( Model *_new );
    ~Model();

    std::shared_ptr<_shader> shader;

private:
    GLuint vertbuff, uvbuff, texbuff;
};

#endif