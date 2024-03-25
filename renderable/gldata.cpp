#include "gldata.h"
#include <iostream>

template<class T, class R>
_Shared<T,R>::_Shared()
{
  //std::cout << "SHAREDUINT DEFAULT CONSTRUCTOR" << std::endl;
  _VAOPTR = nullptr;
}

template<class T, class R>
void _Shared<T,R>::Generate()
{
  _VAOPTR.reset(new T());
}

template<class T, class R>
void _Shared<T,R>::Attach(R _INPUT) {
  _VAOPTR.reset(new T());
  _VAOPTR->_VAL = _INPUT;
}

template<class T, class R>
_Shared<T,R>::operator R() const
{
  return (_VAOPTR) ? _VAOPTR->_VAL : 0;
}

template class _Shared<_SharedVAO,GLuint>;
_SharedVAO::_SharedVAO()
{
  glGenVertexArrays(1, &_VAL);
}

_SharedVAO::~_SharedVAO()
{
  glDeleteVertexArrays(1, &_VAL);
}

template class _Shared<_SharedVBO,GLuint>;
_SharedVBO::_SharedVBO()
{
  glGenBuffers(1, &_VAL);
}

_SharedVBO::~_SharedVBO()
{
  glDeleteBuffers(1, &_VAL);
}

template class _Shared<_SharedTex,GLuint>;
_SharedTex::_SharedTex()
{
  glGenTextures(1, &_VAL);
}

_SharedTex::~_SharedTex()
{
  glDeleteTextures(1, &_VAL);
}

template class _Shared<_SharedSDLWindow,SDL_Window*>;
_SharedSDLWindow::_SharedSDLWindow() {
  ;
}

_SharedSDLWindow::~_SharedSDLWindow() {
  SDL_DestroyWindow(_VAL);
}

template class _Shared<_SharedSDLGLContext,SDL_GLContext>;
_SharedSDLGLContext::_SharedSDLGLContext() {
  ;
}

_SharedSDLGLContext::~_SharedSDLGLContext() {
  SDL_GL_DeleteContext(_VAL);
}
