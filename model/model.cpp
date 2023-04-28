#include "model/model.h"

Model::Model(std::string vertpath, std::string fragpath)
{
    ;
}

Model::~Model()
{
    shader.reset();
    glDeleteBuffers(1,&uvbuff);
    glDeleteBuffers(1,&vertbuff);
}