#ifndef SCENE_H
#define SCENE_H
#pragma once

#include <map>
#include <memory>
#include <string>
#include "shader/shader.h"
#include "model/model.h"
#include "renderable/renderable.h"

class GLScene
{
public:
    GLScene();
    ~GLScene();

//private:
    std::map<std::string, std::shared_ptr<Renderable> > renderables;
    std::map<std::string, std::shared_ptr<Model> > models;
    std::map<std::string, std::shared_ptr<_shader> > shaders;
};

#endif