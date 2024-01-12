#include "origin.h"
#include <gtx/quaternion.hpp>

#define SIZEPERVERT 3

static const float s2D_originVerts[] = {
    -1.f, 0.0f, 0.0f,
     1.f, 0.0f, 0.0f,
    0.0f, -1.f, 0.0f,
     0.0f, 1.f, 0.0f,
    0.0f, 0.0f, -1.f,
     0.0f, 0.0f, 1.f
};

static const float s2D_originColors[] = {
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
};

static const float s3D_originVerts[] = {
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static const float s3D_originColors[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
};

Origin::Origin()
{
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    // Generate a Vertex Buffer Object to represent the cube's vertices
    glGenBuffers(1,&vbuff);
    glGenBuffers(1,&cbuff);

    showColors = true;
    setColors(!showColors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    shader.reset( new _shader() );
    if ( !shader->load("assets/origin.vert","assets/origin.frag") )
    {
        std::cout << "Failed to load shaders!" << std::endl << shader->getErrors() << std::endl;
        return;
    }
    shader->compile();
}

Origin::~Origin()
{
    // Disable location 0 and location 1
    //glDisableVertexArrayAttrib(VAO, 0);
    //glDisableVertexArrayAttrib(VAO, 1);
}

void Origin::setColors(bool enable)
{
    glBindVertexArray(VAO);

    GLsizeiptr vsize, csize;
    const void *vdata, *cdata;

    if (enable == showColors)
        return;

    showColors = enable;

    if (enable)
    {
        vsize = sizeof(s3D_originVerts);
        vdata = s3D_originVerts;

        csize = sizeof(s3D_originColors);
        cdata = s3D_originColors;
    } else {
        vsize = sizeof(s2D_originVerts);
        vdata = s2D_originVerts;

        csize = sizeof(s2D_originColors);
        cdata = s2D_originColors;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbuff);
    glBufferData(GL_ARRAY_BUFFER, vsize, vdata, GL_STATIC_DRAW);
    glVertexAttribPointer(
        0,                  // location
        SIZEPERVERT,        // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, cbuff);
    glBufferData(GL_ARRAY_BUFFER, csize, cdata, GL_STATIC_DRAW);
    glVertexAttribPointer(
        1,                  // location
        SIZEPERVERT,        // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );
}

void Origin::setGrid(bool enable)
{
    ;
}

bool Origin::getColors()
{
     return showColors;   
}
bool Origin::getGrid()
{
    return showGrid;
}

void Origin::render(glm::mat4 projection, glm::mat4 view, float scale)
{
    glBindVertexArray(VAO);

    shader->use();
    shader->setMat4("projection",projection);
    shader->setMat4("view",view);
    shader->setMat4("model",glm::scale(glm::mat4(1.0f),glm::vec3(scale)));

    // Draw the cube
    glDrawArrays(GL_LINES, 0, 6 );
}
