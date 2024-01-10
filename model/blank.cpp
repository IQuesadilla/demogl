#include "blank.h"

Blank::Blank() : Model()
{
    doGenerateMipmap = false;

    CollisionVerts = {0.0f,0.0f,0.0f};

    TCount = 0;
    isEnclosed = false;
    name = "blank";
}

Blank::Blank(Blank *_new)
{
    *this = *_new;
}