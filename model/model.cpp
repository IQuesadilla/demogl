#include "model/model.h"

Model::Model(std::shared_ptr<_shader> shader)
{
    ;
}

Model::Model( Model *_new)
{
    ;
}

Model::~Model()
{
    glDeleteBuffers(1,&uvbuff);
    glDeleteBuffers(1,&vertbuff);
}