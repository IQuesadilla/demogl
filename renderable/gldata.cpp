#include "gldata.h"
#include <iostream>

template<class T>
_SharedUINT<T>::_SharedUINT()
{
  //std::cout << "SHAREDUINT DEFAULT CONSTRUCTOR" << std::endl;
  _VAOPTR = nullptr;
}

template<class T>
void _SharedUINT<T>::Generate()
{
  _VAOPTR.reset(new T());
}

template<class T>
_SharedUINT<T>::operator GLuint() const
{
  return (_VAOPTR) ? _VAOPTR->_VAL : 0;
}

template class _SharedUINT<_SharedVAO>;
_SharedVAO::_SharedVAO()
{
  glGenVertexArrays(1, &_VAL);
}

_SharedVAO::~_SharedVAO()
{
  glDeleteVertexArrays(1, &_VAL);
}

template class _SharedUINT<_SharedVBO>;
_SharedVBO::_SharedVBO()
{
  glGenBuffers(1, &_VAL);
}

_SharedVBO::~_SharedVBO()
{
  glDeleteBuffers(1, &_VAL);
}

template class _SharedUINT<_SharedTex>;
_SharedTex::_SharedTex()
{
  glGenTextures(1, &_VAL);
}

_SharedTex::~_SharedTex()
{
  glDeleteTextures(1, &_VAL);
}
