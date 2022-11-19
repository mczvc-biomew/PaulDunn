#ifndef FRACTAL_RENDERER_HPP
#define FRACTAL_RENDERER_HPP

#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#if defined(GLES2) && defined(__ANDROID__)
  #include <GLES2/gl2.h>
#elif defined(GLES1) && defined(__ANDROID__)
  #include <GLES/gl.h>
#else
  #include "egg2d.h"
#endif

void RendererInit();
void Render();

#endif
