#define GL_GLEXT_PROTOTYPES 1

#include "origin.h"
#include <gtx/quaternion.hpp>

#include "assets/rawcube.h"

Origin::Origin()
{
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    // Generate a Vertex Buffer Object to represent the cube's vertices
    glGenBuffers(1,&vbuff);
    glBindBuffer(GL_ARRAY_BUFFER, vbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originVerts), originVerts, GL_STATIC_DRAW);

    glGenBuffers(1,&cbuff);
    glBindBuffer(GL_ARRAY_BUFFER, cbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(originColors), originColors, GL_STATIC_DRAW);

    shader.reset( new _shader() );
    if ( shader->load("assets/origin.vert","assets/origin.frag") )
    {
        std::cout << "Failed to load shaders!" << std::endl << shader->getErrors() << std::endl;
        return;
    }
}

Origin::~Origin()
{
    ;
}

void Origin::render(glm::mat4 projection, glm::mat4 view, float scale)
{
    glBindVertexArray(VAO);

    shader->use();
    shader->setMat4("projection",projection);
    shader->setMat4("view",view);
    shader->setMat4("model",glm::scale(glm::mat4(1.0f),glm::vec3(scale)));

    glBindBuffer(GL_ARRAY_BUFFER, vbuff);
    glVertexAttribPointer(
        0,                  // location
        3,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );
/*
    glBindBuffer(GL_ARRAY_BUFFER, cbuff);
    glVertexAttribPointer(
        1,                  // location
        3,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );
*/
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);

    // Draw the cube
    glDrawArrays(GL_LINES, 0, 6);

    // Disable location 0 and location 1
    glDisableVertexArrayAttrib(VAO, 0);
    //glDisableVertexArrayAttrib(VAO, 1);
}