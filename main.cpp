#include "demogl.h"

int main()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    demogl dgl;

    std::shared_ptr<button> myb;
    myb.reset(new button("test",0,0,100,100));
    dgl.insertObject(myb);

    dgl.runForever();
    return 0;
}