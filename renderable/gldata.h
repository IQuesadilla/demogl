#pragma once

#include "glad/glad.h"
//#include "glad/khrplatform.h"
#include "SDL.h"
//#if defined __has_include
//    #if __has_include (<SDL_opengl.h>)
//        #include <SDL_opengl.h>
//    #else
//        #include <GL/gl.h>
//    #endif
//#endif

#include <memory>

template <class T, class R>
class _Shared
{
public:
  _Shared();
  void Generate();
  void Attach(R _INPUT);
  operator R() const;
private:
  std::shared_ptr<T> _VAOPTR;
};

class _SharedVAO
{
public:
  _SharedVAO();
  ~_SharedVAO();
  GLuint _VAL;
};
typedef _Shared<_SharedVAO,GLuint> SharedVAO;

class _SharedVBO
{
public:
  _SharedVBO();
  ~_SharedVBO();
  GLuint _VAL;
};
typedef _Shared<_SharedVBO,GLuint> SharedVBO;

class _SharedTex
{
public:
  _SharedTex();
  ~_SharedTex();
  GLuint _VAL;
};
typedef _Shared<_SharedTex,GLuint> SharedTex;

class _SharedSDLWindow {
public:
  _SharedSDLWindow();
  ~_SharedSDLWindow();
  SDL_Window *_VAL;
};
typedef _Shared<_SharedSDLWindow,SDL_Window*> SharedSDLWindow;

class _SharedSDLGLContext {
public:
  _SharedSDLGLContext();
  ~_SharedSDLGLContext();
  SDL_GLContext _VAL;
};
typedef _Shared<_SharedSDLGLContext,SDL_GLContext> SharedSDLGLContext;

//typedef std::shared_ptr<_SharedVAO> SharedVAO;
//#define NewSharedVAO std::make_shared<_SharedVAO>(new _SharedVAO)
