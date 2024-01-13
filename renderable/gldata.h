#pragma once

#include "glad/glad.h"
//#include "glad/khrplatform.h"
//#include "SDL.h"
#if defined __has_include
    #if __has_include (<SDL_opengl.h>)
        #include <SDL_opengl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif

#include <memory>

template <class T>
class _SharedUINT
{
public:
  _SharedUINT();
  void Generate();
  operator GLuint() const;
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
template class _SharedUINT<_SharedVAO>;
typedef _SharedUINT<_SharedVAO> SharedVAO;

class _SharedVBO
{
public:
  _SharedVBO();
  ~_SharedVBO();
  GLuint _VAL;
};
template class _SharedUINT<_SharedVBO>;
typedef _SharedUINT<_SharedVBO> SharedVBO;

class _SharedTex
{
public:
  _SharedTex();
  ~_SharedTex();
  GLuint _VAL;
};
template class _SharedUINT<_SharedTex>;
typedef _SharedUINT<_SharedTex> SharedTex;

//typedef std::shared_ptr<_SharedVAO> SharedVAO;
//#define NewSharedVAO std::make_shared<_SharedVAO>(new _SharedVAO)
