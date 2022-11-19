#include <cstdio>
#include <SDL2/SDL.h>

#ifdef GLES2
#define HAVE_OPENGLES2
#endif

#ifdef HAVE_OPENGLES2
#include "fractal_renderer.hpp"

/**
 * SDL2 on Android and Linux. <li> Cxxdroid </li>
 * <li> CMake </li> <li> SDL2 </li> <li> SDL2_image </li> <li> GLAD </li>
 * <li> Relminator: <u>http://rel.phatcode.net<u> </li> <li> PaulDunn: SpecBasic </li>
 * <li> Jetbrain's Clion <br>
 * ### [mczvc-biomew] \<czarm827\@protonmail.com\><br>
 * Meldencio Czarlemagne Veras Corrales, BSCS
 * <li> Easy Game Graphics, Egg Co. </li>
 */
//namespace {

	int g_targetWidth = 1920;
	int g_targetHeight = 1100;
	
	double g_targetAspect = g_targetWidth / static_cast<double>(g_targetHeight);

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
