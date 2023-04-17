#ifndef DEMOGL_H
#define DEMOGL_H

#include "SDL.h"
#include <memory>
#include <string>
#include <vector>
#include <iostream>

class object
{
public:
    object(std::string name, int x, int y, int w, int h);
    ~object();

    bool testIntersection(SDL_Point v);

    virtual void mDefault() = 0;
    virtual void mHover() = 0;
    virtual void mClick() = 0;
    virtual void mHold() = 0;

    void setRenderer(SDL_Renderer *r);

protected:
    SDL_Renderer *_renderer;
    std::string _name;
    SDL_Rect clip;
};

class button : public object
{
public:
    button(std::string name, int x, int y, int w, int h) : object(name,x,y,w,h) {}

    void mDefault();
    void mHover();
    void mClick();
    void mHold();
};

class demogl
{
public:
    demogl();
    ~demogl();

    void runForever();

    void insertObject(std::shared_ptr<object> o);

private:
    void pollInput();
    void updateWindow();

    struct {
        bool runNextLoop;
    } flags;

    SDL_Point mPos;

    std::vector<std::shared_ptr<object> > _objects;
    SDL_Window *_window;
    SDL_Renderer *_renderer;
};

#endif