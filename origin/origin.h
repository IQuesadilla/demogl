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

    void setColors(bool enable);
    void setGrid(bool enable);

    bool getColors();
    bool getGrid();

    void render(glm::mat4 projection, glm::mat4 view, float model);

private:
    std::shared_ptr<_shader> shader;
    GLuint VAO, vbuff, cbuff;

    bool showColors;
    bool showGrid;
};

#endif