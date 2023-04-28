#ifndef ORIGIN_H
#define ORIGIN_H
#pragma once

#include "shader/shader.h"
#include <memory>

class Origin
{
public:
    Origin();
    ~Origin();

    void render(glm::mat4 projection, glm::mat4 view);

private:
    std::shared_ptr<_shader> shader;
    GLuint VAO, vbuff, cbuff;
};

#endif