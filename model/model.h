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
    Model( Model *_new );
    ~Model();

    void bind();

    std::shared_ptr<_shader> shader;
    GLuint VAO;

protected:
    Model() {};
    void setModel(std::vector<GLfloat> vertData, std::vector<GLfloat> uvData);

private:
    GLuint vertbuff, uvbuff, texbuff;
};

#endif