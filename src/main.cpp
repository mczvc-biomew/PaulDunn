#include <cstdio>
#include <SDL2/SDL.h>

#ifndef HAVE_OPENGLES2
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
	int ww, wh;
	int nw, nh;
	
	int g_targetWidth = 1920;
	int g_targetHeight = 1100;
	
	float aspect;
	double g_targetAspect = g_targetWidth / static_cast<float>(g_targetHeight);
	
	float scale = 1.0f;
	float cropH, cropV;
	
//	SDL_Window* window;
//};

void init_renderer(int originX, int originY, int width, int height) {
#if 0
	glViewport(originX, originY, width, height);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	SDL_GL_SwapWindow(window);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
}

int init_window_x_renderer() {
#if 0
	window = SDL_CreateWindow("Bubble Universe",
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		targetWidth, targetHeight, SDL_WINDOW_OPENGL);
	
	if (!window)
	{
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return -1;
	}

	// We will not actually need a context created, but we should create one
	SDL_GLContext gl = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl);
	
	// Initialize glad
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    	fprintf(stderr, "Cannot load GLAD\n");
    	return -1;
    }
	
	SDL_GetWindowSize(window, &ww, &wh);
	
	aspect = static_cast<float>(ww) / wh;
	cropH = 0.f, cropV = 0.f;
	
	if (aspect > targetAspect) {
		scale = static_cast<float>(wh) / targetHeight;
		cropH = (ww - targetWidth * scale) * 0.5f;
	} else if (aspect < targetAspect) {
		scale = static_cast<float>(ww) / targetWidth;
		cropV = (wh - targetHeight * scale) * 0.5f;
	} else { scale = static_cast<float>(ww) / targetWidth; }
	
	nw = targetWidth * scale;
	nh = targetHeight * scale;
	
	init_renderer(cropH, cropV, nw, nh);
	return 0;
#endif
}

int init() {
#if 0
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	// We use OpenGL ES 2.0
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// We want at least 8 bits per color
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    SDL_GL_SetSwapInterval(1);
	return 0;
#endif
}

bool paused = false;

int main(int argc, char *argv[])
{
	bool quit = false;
//	check_return(init());
//	check_return(init_window_x_renderer());
    createWindow("Bubble Universe");
	
	// Initialize triangle renderer
//	renderer_init();
    renderInit3();

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
        unsigned int currentTick = SDL_GetTicks();
        double deltaTime = (double) (currentTick - lastTick) / 1000.0;
        lastTick = currentTick;

        Update(deltaTime);
		// Draw triangle
		Render();
	}

    GL2D_Quit();
//	SDL_DestroyWindow(window);
//	SDL_Quit();
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
