#include "demogl.h"

object::object(std::string name, int x, int y, int w, int h)
{
    _name = name;
    clip.x = x;
    clip.y = y;
    clip.w = w;
    clip.h = h;
}

object::~object()
{
    ;
}

bool object::testIntersection(SDL_Point v)
{
    return  v.x >= clip.x && v.x <= clip.x + clip.w &&
            v.y >= clip.y && v.y <= clip.y + clip.h ;
}

void object::setRenderer(SDL_Renderer *r)
{
    _renderer = r;
}

void button::mDefault()
{
    SDL_SetRenderDrawColor(_renderer,0,0,0,0);
    SDL_RenderFillRect(_renderer,&clip);

    SDL_SetRenderDrawColor(_renderer,0,0,255,255);
    SDL_RenderDrawRect(_renderer,&clip);
}

void button::mHover()
{
    SDL_SetRenderDrawColor(_renderer,255,255,255,255);
    SDL_RenderFillRect(_renderer,&clip);
}

void button::mClick()
{
    std::cout << _name << ": click" << std::endl;
    mHover();
}

void button::mHold()
{
    SDL_SetRenderDrawColor(_renderer,100,100,100,255);
    SDL_RenderFillRect(_renderer,&clip);
}

demogl::demogl()
{
    _window = SDL_CreateWindow( "DEMOGL",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                640, 480,
                                SDL_WINDOW_SHOWN );

    _renderer = SDL_CreateRenderer( _window, -1, SDL_RENDERER_ACCELERATED);

    flags.runNextLoop = true;
}

demogl::~demogl()
{
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void demogl::runForever()
{
    for (auto &x : _objects)
        x->mDefault();
    updateWindow();

    while ( flags.runNextLoop )
    {
        pollInput();
        updateWindow();
    }
}

void demogl::insertObject(std::shared_ptr<object> o)
{
    o->setRenderer(_renderer);
    _objects.push_back(o);
}

void demogl::pollInput()
{
    SDL_Event e;
    Uint32 clickState;
    SDL_WaitEvent(&e);
        switch (e.type)
        {
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&mPos.x,&mPos.y);

                for (auto &x : _objects)
                    if (x->testIntersection(mPos))
                        x->mClick();
                    else x->mDefault();
            break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEMOTION:
                clickState = SDL_GetMouseState(&mPos.x,&mPos.y);

                for (auto &x : _objects)
                    if (x->testIntersection(mPos))
                        if (clickState & SDL_BUTTON_LMASK)
                            x->mHold();
                        else
                            x->mHover();
                    else x->mDefault();
            break;

            case SDL_QUIT:
                flags.runNextLoop = false;
            break;
        }
}

void demogl::updateWindow()
{
    SDL_SetRenderDrawColor(_renderer,0,0,0,0);
    SDL_RenderPresent(_renderer);
}