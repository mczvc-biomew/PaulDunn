#ifndef FRACTAL_RENDERER_HPP
#define FRACTAL_RENDERER_HPP

#include <math.h>
//#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#if defined(GLES2) && defined(Android)
  #include <GLES2/gl2.h>
#elif defined(GLES1) && defined(Android)
  #include <GLES/gl.h>
#else
  #include <glad/glad.h>
#endif

void renderer_init();
void render(SDL_Window *window);

#endif
