#ifndef FRACTAL_RENDERER_HPP
#define FRACTAL_RENDERER_HPP

#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#if defined(GLES2) && defined(ANDROID)
  #include <GLES2/gl2.h>
#elif defined(GLES1) && defined(ANDROID)
  #include <GLES/gl.h>
#else
extern "C" {
   #include "gl2d.h"
}
#endif

void renderer_init();
void render();

#endif
