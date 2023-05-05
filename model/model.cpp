#include "model/model.h"

void Model::setModel(std::vector<GLfloat> vertData, std::vector<GLfloat> uvData)
{
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

/*
    SDL_Surface *icon = SDL_LoadBMP("assets/uvtemplate.bmp");
    if (icon == NULL)
    {
        std::cout << "Failed to load image" << std::endl;
        return;
    }


    glGenTextures(1, &texbuff);
    glBindTexture(GL_TEXTURE_2D, texbuff);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    GLuint format, iformat;
    if (icon->format->BitsPerPixel == 32)
    {
        format = GL_BGRA;
        iformat = GL_RGBA;
    }
    else
    {
        format = GL_BGR;
        iformat = GL_RGB;
    }

    glTexImage2D( GL_TEXTURE_2D, 0, iformat, icon->w, icon->h, 0, format, GL_UNSIGNED_BYTE, icon->pixels );

    SDL_FreeSurface(icon);

    glGenerateMipmap(GL_TEXTURE_2D);
*/
    // Generate a Vertex Buffer Object to represent the cube's vertices
    glGenBuffers(1,&vertbuff);
    glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
    glBufferData(GL_ARRAY_BUFFER, vertData.size() * sizeof(GLfloat), vertData.data(), GL_STATIC_DRAW);

    // Generate a Vertex Buffer Object to represent the cube's colors
    glGenBuffers(1, &uvbuff);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
    glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(GLfloat), uvData.data(), GL_STATIC_DRAW);

            // Set vertices to location 0 - GLSL: layout(location = 0) in vec3 aPos;
    glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
    glVertexAttribPointer(
        0,                  // location
        3,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );

    // Set colors to location 1 - GLSL: layout(location = 1) in vec3 aColor;
    glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
    glVertexAttribPointer(
        1,                  // location
        3,                  // size (per vertex)
        GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
        GL_FALSE,           // is normalized*
        0,                  // stride**
        (void*)0            // array buffer offset
    );

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, texbuff);
}

Model::Model( Model *_new)
{
    *this = *_new;
}

Model::~Model()
{
    glDeleteBuffers(1,&uvbuff);
    glDeleteBuffers(1,&vertbuff);
    glDeleteVertexArrays(1,&VAO);
}

void Model::bind()
{
    glBindVertexArray(VAO);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, texbuff);

}