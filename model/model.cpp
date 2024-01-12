#include "model/model.h"

Model::Model()
{
    Init();
}

Model::Model( Model *_new)
{
    *this = *_new;
}

Model::~Model()
{
    glBindVertexArray(0); // Unbind the VAO
    glDeleteBuffers(1,&uvbuff);
    glDeleteBuffers(1,&vertbuff);
    glDeleteVertexArrays(1,&VAO);
}

void Model::GLInit()
{
  updateModel(_UnloadedModel->vertices);

  delete _UnloadedModel;
  _UnloadedModel = nullptr;
}

void Model::bind()
{
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texbuff);

}

void Model::draw()
{
  if (_UnloadedModel) GLInit();
    if (!ibuff)
        glDrawArrays(GL_TRIANGLES, 0, 3 * TCount);
    else
        glDrawElements(GL_TRIANGLES, 3 * TCount, GL_UNSIGNED_INT, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "Model \"" << name << "\" Error:" << err << std::endl;
    }
}

void Model::Init()
{
    vertbuff = 0;
    ibuff = 0;
    uvbuff = 0;
    texbuff = 0;
    VAO = 0;
    shader = nullptr;
    _UnloadedModel = new UnloadedModel;
}

void Model::setModel(std::vector<GLfloat> vertData)
{
    std::vector<glm::vec3> NewVertData;
    NewVertData.reserve(vertData.size() / 3);
    for (auto it = vertData.begin(); it < vertData.end(); it += 3)
        NewVertData.push_back({it[0],it[1],it[2]});
    setModel(NewVertData);
}

void Model::setModel(std::vector<glm::vec3> vertData)
{
  if (_UnloadedModel)
    _UnloadedModel->vertices = vertData;
  else
    updateModel(vertData);
}

void Model::updateModel(std::vector<glm::vec3> vertData)
{
    if (!VAO) glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    //SDL_Surface *icon = SDL_LoadBMP( IMGPath.c_str() );
    //if (icon == NULL)
    //{
    //    std::cout << "Failed to load image" << std::endl;
    //    return;
    //}

    /*GLuint format, iformat;
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

    SDL_FreeSurface(icon);*/

    if (vertbuff == 0)
    {
        // Generate a Vertex Buffer Object to represent the cube's vertices
        glGenBuffers(1,&vertbuff);
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
        glEnableVertexAttribArray(0);
    }

    if (vertData.size() > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertbuff);
        glBufferData(GL_ARRAY_BUFFER, vertData.size() * sizeof(glm::vec3), vertData.data(), GL_STATIC_DRAW);
    }
}

void Model::setIndices(std::vector<GLuint> indexData)
{
    if (!VAO) glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    if (!ibuff) glGenBuffers(1, &ibuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);
}

void Model::setTex(cv::Mat image, std::vector<GLfloat> uvData)
{
    if (!VAO) glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    if (texbuff == 0)
    {
        glGenTextures(1, &texbuff);
        glBindTexture(GL_TEXTURE_2D, texbuff);

        if ( doGenerateMipmap )
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    if (uvbuff == 0)
    {
        glGenBuffers(1, &uvbuff);
        // Set colors to location 1 - GLSL: layout(location = 1) in vec3 aColor;
        glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
        glVertexAttribPointer(
            2,                  // location
            2,                  // size (per vertex)
            GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
            GL_FALSE,           // is normalized*
            0,                  // stride**
            (void*)0            // array buffer offset
        );
        glEnableVertexAttribArray(2);

        // * if normalized is set to GL_TRUE, it indicates that values stored in an integer format are to be mapped
        // * to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted
        // * to floating point. Otherwise, values will be converted to floats directly without normalization. 

        // ** 0 means tightly packed, in this case 0 means OpenGL should automatically calculate size * sizeof(GLFloat) = 12
        // ** no distance from "how many bytes it is from the start of one element to the start of another"

    }

    if (uvData.size() > 0)
    {
        // Generate a Vertex Buffer Object to represent the cube's colors
        glBindBuffer(GL_ARRAY_BUFFER, uvbuff);
        glBufferData(GL_ARRAY_BUFFER, uvData.size() * sizeof(GLfloat), uvData.data(), GL_STATIC_DRAW);
    }

    if (!image.empty())
    {
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

        if (!image.isContinuous()) {
            image = image.clone(); // Ensure the data is continuous
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data ); 

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "Model Error:" << err << std::endl;
        }

        if ( doGenerateMipmap )
            glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void Model::setColors(std::vector<GLfloat> colorData)
{
    if (colorbuff == 0)
    {
        glGenBuffers(1, &colorbuff);
        // Set colors to location 1 - GLSL: layout(location = 1) in vec3 aColor;
        glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
        glVertexAttribPointer(
            1,                  // location
            3,                  // size (per vertex)
            GL_FLOAT,           // type (32-bit float, equal to C type GLFloat)
            GL_FALSE,           // is normalized*
            0,                  // stride**
            (void*)0            // array buffer offset
        );
        glEnableVertexAttribArray(1);
    }

    if (colorData.size() > 0)
    {
        // Generate a Vertex Buffer Object to represent the cube's colors
        glBindBuffer(GL_ARRAY_BUFFER, colorbuff);
        glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(GLfloat), colorData.data(), GL_STATIC_DRAW);
    }
}
