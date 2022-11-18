#include <cstdio>
#include <SDL2/SDL.h>

#ifdef GLES2
#define HAVE_OPENGLES2
#endif

#ifdef HAVE_OPENGLES2
#include "fractal_renderer.hpp"

#define check_return(x) if ((x) != 0) return 1
#define GL_CHECK(x) \ 
    x; \
    { \
      GLenum glError = glGetError(); \
      if(glError != GL_NO_ERROR) { \
        SDL_Log("glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
        return 1; \
      } \
    }

//namespace {

	int g_targetWidth = 1920;
	int g_targetHeight = 1100;
	
	float aspect;
	double g_targetAspect = g_targetWidth / static_cast<float>(g_targetHeight);

//};

bool paused = false;

int main(int argc, char *argv[])
{
	bool quit = createWindow("Bubble Universe") != 0;

	RendererInit();
//    renderInit3();

    unsigned int lastTick = 0;

	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// Process events so the app doesn't hang
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				SDL_Log("Quitting..");
        		quit = true;
        	} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
        		paused = !paused;
        	} 
		}
//        unsigned int currentTick = SDL_GetTicks();
//        double deltaTime = (double) (currentTick - lastTick) / 1000.0;
//        lastTick = currentTick;

		// Draw the fractal
		Render();
	}

    EGG_Quit();
	return 0;
}

#else
int
main(int argc, char *argv[])
{
    SDL_Log("No OpenGL ES support on this system\n");
    return 1;
}

#endif /* HAVE_OPENGLES2 */
