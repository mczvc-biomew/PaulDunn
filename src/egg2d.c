/**
 * @file egg2d.c
 * Easy Game Graphics 0.1
 *
 * [mczvc] (2022) <czarm827@protonmail.com>
 * Meldencio Czarlemagne Veras Corrales, CS (2nd year)
 *
 * https://github.com/mczvc827
 */

#include "egg2d.h"
#include <SDL2/SDL_image.h>

static GLuint gCurrentTexture = 0;

static SDL_Surface *screenSurface = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

/**
 * Init OpenGL viewport
 * @param originX the viewport origin on left
 * @param originY the viewport origin on top
 * @param width the viewport's width
 * @param height the viewport' height
 */
static void initViewPort(int originX, int originY, int width, int height) {
    glViewport(originX, originY, width, height);
    gCurrentTexture = 0;
}

/**
 * Initialized OpenGL default 2D
 */
static void init2D() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearDepth(1.0);                       //'Set Depth buffer to 1(z-Buffer)
    glDisable(GL_DEPTH_TEST);                //'Disable Depth Testing so that our z-buffer works

    // compare each incoming pixel z value with the z value present in the depth buffer
    // LEQUAL means than pixel is drawn if the incoming z value is less than
    //or equal to the stored z value
    glDepthFunc(GL_LEQUAL);

//    'have one or more material parameters track the current color
//    'Material is your 3d model
//    glEnable(GL_COLOR_MATERIAL);

    //Enable Texturing
    glEnable(GL_TEXTURE_2D);

    glPolygonMode(GL_FRONT, GL_FILL);

//    'Disable Backface culling
    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_DITHER);

    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

}

/**
 * Creates a new Window. Defaults to SDL window.
 * @param title The title of the window.
 * @return -1: on error; 0 on success.
 */
int createWindow(const char* title) {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetSwapInterval(1);

    window = SDL_CreateWindow(title, 0, 0,
                              g_targetWidth, g_targetHeight, SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // We will not actually need a context created, but we should create one
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Cannot load GLAD\n");
        return -1;
    }

    SDL_GetWindowSize(window, &g_actualWidth, &g_actualHeight);

    g_aspect = (float)g_actualWidth / g_actualHeight;
    g_cropH = 0.f; g_cropV = 0.0f;

    if (g_aspect > g_targetAspect) {
        g_scale = (float)g_actualWidth / g_targetHeight;
        g_cropH = (g_actualWidth - g_targetWidth * g_scale) * 0.5f;
    } else if (g_aspect < g_targetAspect) {
        g_scale = (float)g_actualWidth / g_targetWidth;
        g_cropV = (g_actualHeight - g_targetHeight * g_scale) * 0.5f;
    } else { g_scale = (float)g_actualWidth / g_targetWidth; }

    g_scaledWidth = g_targetWidth * g_scale;
    g_scaledHeight = g_targetHeight * g_scale;

    // Initialize viewport and shaders
    initViewPort(g_cropH, g_cropV, g_scaledWidth, g_scaledHeight);

    //printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));

    screenSurface = SDL_GetWindowSurface(window);
    if (!screenSurface) {
        fprintf(stderr, "could not get window surface: %s\n", SDL_GetError());
        return -1;
    }

    init2D();

    return 0;
}

void EGG_Quit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

GLuint loadShader(GLenum type, const char* shaderSrc) {
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);

    if (shader == 0) {
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if ( infoLen > -1 ) {
            char *infoLog = (char*) malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            fprintf(stderr, "%s", infoLog);

            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint loadShaderProgram(const char *vertexShaderSrc, const char *fragShaderSrc) {
    GLint linked;
    // Load shaders
    GLuint vertexShaderObj = loadShader(GL_VERTEX_SHADER, vertexShaderSrc);

    if (vertexShaderObj == 0) {
        fprintf(stderr, "There's an error compiling the vertexShaderObj shader.\n");
        return 0;
    }

    GLuint fragmentShaderObj = loadShader(GL_FRAGMENT_SHADER, fragShaderSrc);

    if (fragmentShaderObj == 0) {
        fprintf(stderr, "There's an error compiling the fragmentShaderObj shader.\n");
        return 0;
    }

    // Combine shaders into shaderProgramObj
    GLuint shaderProgramObj = glCreateProgram();

    if (shaderProgramObj == 0) {
        fprintf(stderr, "There's an error creating shader shaderProgramObj.\n");
        return 0;
    }

    glAttachShader(shaderProgramObj, vertexShaderObj);
    glAttachShader(shaderProgramObj, fragmentShaderObj);
    // Link the shaderProgramObj
    glLinkProgram(shaderProgramObj);

    glGetProgramiv(shaderProgramObj, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(shaderProgramObj, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > -1) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(shaderProgramObj, infoLen, NULL, infoLog);
            fprintf(stderr, "Error linking shaderProgramObj\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteProgram(shaderProgramObj);
        return 0;
    }

    glDeleteShader(vertexShaderObj);
    glDeleteShader(fragmentShaderObj);

    return shaderProgramObj;
}


GLuint GetGlowImage() {

    static GLuint textureID = 0;

    if (textureID == 0) {
        const int IMAGE_WIDTH = 32, IMAGE_HEIGHT = 32;

        const unsigned int image_array[] = {
                0x00000007, 0x00000004, 0x00000020, 0x00000020, 0x00000080, 0x00000000,
                0x00000000, 0x00000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF010101, 0xFF020202,
                0xFF020202, 0xFF030303, 0xFF030303, 0xFF030303, 0xFF020202, 0xFF020202,
                0xFF010101, 0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF020202, 0xFF030303,
                0xFF040404, 0xFF050505, 0xFF060606, 0xFF060606, 0xFF060606, 0xFF060606,
                0xFF060606, 0xFF050505, 0xFF040404, 0xFF030303, 0xFF020202, 0xFF010101,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF020202, 0xFF040404,
                0xFF050505, 0xFF070707, 0xFF080808, 0xFF090909, 0xFF0A0A0A, 0xFF0B0B0B,
                0xFF0B0B0B, 0xFF0B0B0B, 0xFF0A0A0A, 0xFF090909, 0xFF080808, 0xFF070707,
                0xFF050505, 0xFF040404, 0xFF020202, 0xFF010101, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF030303,
                0xFF050505, 0xFF070707, 0xFF090909, 0xFF0B0B0B, 0xFF0C0C0C, 0xFF0E0E0E,
                0xFF0F0F0F, 0xFF0F0F0F, 0xFF101010, 0xFF0F0F0F, 0xFF0F0F0F, 0xFF0E0E0E,
                0xFF0C0C0C, 0xFF0B0B0B, 0xFF090909, 0xFF070707, 0xFF050505, 0xFF030303,
                0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101,
                0xFF030303, 0xFF060606, 0xFF080808, 0xFF0B0B0B, 0xFF0D0D0D, 0xFF0F0F0F,
                0xFF111111, 0xFF131313, 0xFF141414, 0xFF151515, 0xFF151515, 0xFF151515,
                0xFF141414, 0xFF131313, 0xFF111111, 0xFF0F0F0F, 0xFF0D0D0D, 0xFF0B0B0B,
                0xFF080808, 0xFF060606, 0xFF030303, 0xFF010101, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF010101, 0xFF030303, 0xFF060606, 0xFF090909, 0xFF0C0C0C, 0xFF0E0E0E,
                0xFF121212, 0xFF141414, 0xFF161616, 0xFF181818, 0xFF191919, 0xFF1A1A1A,
                0xFF1B1B1B, 0xFF1A1A1A, 0xFF191919, 0xFF181818, 0xFF161616, 0xFF141414,
                0xFF121212, 0xFF0E0E0E, 0xFF0C0C0C, 0xFF090909, 0xFF060606, 0xFF030303,
                0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF010101, 0xFF030303, 0xFF060606, 0xFF090909, 0xFF0C0C0C,
                0xFF0F0F0F, 0xFF131313, 0xFF161616, 0xFF191919, 0xFF1B1B1B, 0xFF1E1E1E,
                0xFF1F1F1F, 0xFF202020, 0xFF212121, 0xFF202020, 0xFF1F1F1F, 0xFF1E1E1E,
                0xFF1B1B1B, 0xFF191919, 0xFF161616, 0xFF131313, 0xFF0F0F0F, 0xFF0C0C0C,
                0xFF090909, 0xFF060606, 0xFF030303, 0xFF010101, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF020202, 0xFF050505, 0xFF080808,
                0xFF0C0C0C, 0xFF0F0F0F, 0xFF131313, 0xFF171717, 0xFF1B1B1B, 0xFF1E1E1E,
                0xFF212121, 0xFF232323, 0xFF252525, 0xFF262626, 0xFF272727, 0xFF262626,
                0xFF252525, 0xFF232323, 0xFF212121, 0xFF1E1E1E, 0xFF1B1B1B, 0xFF171717,
                0xFF131313, 0xFF0F0F0F, 0xFF0C0C0C, 0xFF080808, 0xFF050505, 0xFF020202,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF040404,
                0xFF070707, 0xFF0B0B0B, 0xFF0E0E0E, 0xFF131313, 0xFF171717, 0xFF1B1B1B,
                0xFF1F1F1F, 0xFF232323, 0xFF262626, 0xFF292929, 0xFF2B2B2B, 0xFF2C2C2C,
                0xFF2C2C2C, 0xFF2C2C2C, 0xFF2B2B2B, 0xFF292929, 0xFF262626, 0xFF232323,
                0xFF1F1F1F, 0xFF1B1B1B, 0xFF171717, 0xFF131313, 0xFF0E0E0E, 0xFF0B0B0B,
                0xFF070707, 0xFF040404, 0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF020202, 0xFF050505, 0xFF090909, 0xFF0D0D0D, 0xFF121212, 0xFF161616,
                0xFF1B1B1B, 0xFF1F1F1F, 0xFF242424, 0xFF282828, 0xFF2B2B2B, 0xFF2E2E2E,
                0xFF313131, 0xFF323232, 0xFF323232, 0xFF323232, 0xFF313131, 0xFF2E2E2E,
                0xFF2B2B2B, 0xFF282828, 0xFF242424, 0xFF1F1F1F, 0xFF1B1B1B, 0xFF161616,
                0xFF121212, 0xFF0D0D0D, 0xFF090909, 0xFF050505, 0xFF020202, 0xFF000000,
                0xFF000000, 0xFF010101, 0xFF030303, 0xFF070707, 0xFF0B0B0B, 0xFF0F0F0F,
                0xFF141414, 0xFF191919, 0xFF1E1E1E, 0xFF232323, 0xFF282828, 0xFF2C2C2C,
                0xFF303030, 0xFF333333, 0xFF363636, 0xFF383838, 0xFF383838, 0xFF383838,
                0xFF363636, 0xFF333333, 0xFF303030, 0xFF2C2C2C, 0xFF282828, 0xFF232323,
                0xFF1E1E1E, 0xFF191919, 0xFF141414, 0xFF0F0F0F, 0xFF0B0B0B, 0xFF070707,
                0xFF030303, 0xFF010101, 0xFF000000, 0xFF010101, 0xFF040404, 0xFF080808,
                0xFF0C0C0C, 0xFF111111, 0xFF161616, 0xFF1B1B1B, 0xFF212121, 0xFF262626,
                0xFF2B2B2B, 0xFF303030, 0xFF343434, 0xFF383838, 0xFF3B3B3B, 0xFF3D3D3D,
                0xFF3D3D3D, 0xFF3D3D3D, 0xFF3B3B3B, 0xFF383838, 0xFF343434, 0xFF303030,
                0xFF2B2B2B, 0xFF262626, 0xFF212121, 0xFF1B1B1B, 0xFF161616, 0xFF111111,
                0xFF0C0C0C, 0xFF080808, 0xFF040404, 0xFF010101, 0xFF000000, 0xFF020202,
                0xFF050505, 0xFF090909, 0xFF0E0E0E, 0xFF131313, 0xFF181818, 0xFF1E1E1E,
                0xFF232323, 0xFF292929, 0xFF2E2E2E, 0xFF333333, 0xFF383838, 0xFF3C3C3C,
                0xFF3F3F3F, 0xFF424242, 0xFF424242, 0xFF424242, 0xFF3F3F3F, 0xFF3C3C3C,
                0xFF383838, 0xFF333333, 0xFF2E2E2E, 0xFF292929, 0xFF232323, 0xFF1E1E1E,
                0xFF181818, 0xFF131313, 0xFF0E0E0E, 0xFF090909, 0xFF050505, 0xFF020202,
                0xFF000000, 0xFF020202, 0xFF060606, 0xFF0A0A0A, 0xFF0F0F0F, 0xFF141414,
                0xFF191919, 0xFF1F1F1F, 0xFF252525, 0xFF2B2B2B, 0xFF313131, 0xFF363636,
                0xFF3B3B3B, 0xFF3F3F3F, 0xFF434343, 0xFF454545, 0xFF474747, 0xFF454545,
                0xFF434343, 0xFF3F3F3F, 0xFF3B3B3B, 0xFF363636, 0xFF313131, 0xFF2B2B2B,
                0xFF252525, 0xFF1F1F1F, 0xFF191919, 0xFF141414, 0xFF0F0F0F, 0xFF0A0A0A,
                0xFF060606, 0xFF020202, 0xFF000000, 0xFF030303, 0xFF060606, 0xFF0B0B0B,
                0xFF0F0F0F, 0xFF151515, 0xFF1A1A1A, 0xFF202020, 0xFF262626, 0xFF2C2C2C,
                0xFF323232, 0xFF383838, 0xFF3D3D3D, 0xFF424242, 0xFF454545, 0xFFAEAEAE,
                0xFFD3D3D3, 0xFF454545, 0xFF454545, 0xFF424242, 0xFF3D3D3D, 0xFF383838,
                0xFF323232, 0xFF2C2C2C, 0xFF262626, 0xFF202020, 0xFF1A1A1A, 0xFF151515,
                0xFF0F0F0F, 0xFF0B0B0B, 0xFF060606, 0xFF030303, 0xFF000000, 0xFF030303,
                0xFF060606, 0xFF0B0B0B, 0xFF101010, 0xFF151515, 0xFF1B1B1B, 0xFF212121,
                0xFF272727, 0xFF2C2C2C, 0xFF323232, 0xFF383838, 0xFF3D3D3D, 0xFF424242,
                0xFF474747, 0xFFD3D3D3, 0xFFFFFFFF, 0xFF454545, 0xFF474747, 0xFF424242,
                0xFF3D3D3D, 0xFF383838, 0xFF323232, 0xFF2C2C2C, 0xFF272727, 0xFF212121,
                0xFF1B1B1B, 0xFF151515, 0xFF101010, 0xFF0B0B0B, 0xFF060606, 0xFF030303,
                0xFF000000, 0xFF030303, 0xFF060606, 0xFF0B0B0B, 0xFF0F0F0F, 0xFF151515,
                0xFF1A1A1A, 0xFF202020, 0xFF262626, 0xFF2C2C2C, 0xFF323232, 0xFF383838,
                0xFF3D3D3D, 0xFF424242, 0xFF454545, 0xFF454545, 0xFF454545, 0xFF454545,
                0xFF454545, 0xFF424242, 0xFF3D3D3D, 0xFF383838, 0xFF323232, 0xFF2C2C2C,
                0xFF262626, 0xFF202020, 0xFF1A1A1A, 0xFF151515, 0xFF0F0F0F, 0xFF0B0B0B,
                0xFF060606, 0xFF030303, 0xFF000000, 0xFF020202, 0xFF060606, 0xFF0A0A0A,
                0xFF0F0F0F, 0xFF141414, 0xFF191919, 0xFF1F1F1F, 0xFF252525, 0xFF2B2B2B,
                0xFF313131, 0xFF363636, 0xFF3B3B3B, 0xFF3F3F3F, 0xFF434343, 0xFF454545,
                0xFF474747, 0xFF454545, 0xFF434343, 0xFF3F3F3F, 0xFF3B3B3B, 0xFF363636,
                0xFF313131, 0xFF2B2B2B, 0xFF252525, 0xFF1F1F1F, 0xFF191919, 0xFF141414,
                0xFF0F0F0F, 0xFF0A0A0A, 0xFF060606, 0xFF020202, 0xFF000000, 0xFF020202,
                0xFF050505, 0xFF090909, 0xFF0E0E0E, 0xFF131313, 0xFF181818, 0xFF1E1E1E,
                0xFF232323, 0xFF292929, 0xFF2E2E2E, 0xFF333333, 0xFF383838, 0xFF3C3C3C,
                0xFF3F3F3F, 0xFF424242, 0xFF424242, 0xFF424242, 0xFF3F3F3F, 0xFF3C3C3C,
                0xFF383838, 0xFF333333, 0xFF2E2E2E, 0xFF292929, 0xFF232323, 0xFF1E1E1E,
                0xFF181818, 0xFF131313, 0xFF0E0E0E, 0xFF090909, 0xFF050505, 0xFF020202,
                0xFF000000, 0xFF010101, 0xFF040404, 0xFF080808, 0xFF0C0C0C, 0xFF111111,
                0xFF161616, 0xFF1B1B1B, 0xFF212121, 0xFF262626, 0xFF2B2B2B, 0xFF303030,
                0xFF343434, 0xFF383838, 0xFF3B3B3B, 0xFF3D3D3D, 0xFF3D3D3D, 0xFF3D3D3D,
                0xFF3B3B3B, 0xFF383838, 0xFF343434, 0xFF303030, 0xFF2B2B2B, 0xFF262626,
                0xFF212121, 0xFF1B1B1B, 0xFF161616, 0xFF111111, 0xFF0C0C0C, 0xFF080808,
                0xFF040404, 0xFF010101, 0xFF000000, 0xFF010101, 0xFF030303, 0xFF070707,
                0xFF0B0B0B, 0xFF0F0F0F, 0xFF141414, 0xFF191919, 0xFF1E1E1E, 0xFF232323,
                0xFF282828, 0xFF2C2C2C, 0xFF303030, 0xFF333333, 0xFF363636, 0xFF383838,
                0xFF383838, 0xFF383838, 0xFF363636, 0xFF333333, 0xFF303030, 0xFF2C2C2C,
                0xFF282828, 0xFF232323, 0xFF1E1E1E, 0xFF191919, 0xFF141414, 0xFF0F0F0F,
                0xFF0B0B0B, 0xFF070707, 0xFF030303, 0xFF010101, 0xFF000000, 0xFF000000,
                0xFF020202, 0xFF050505, 0xFF090909, 0xFF0D0D0D, 0xFF121212, 0xFF161616,
                0xFF1B1B1B, 0xFF1F1F1F, 0xFF242424, 0xFF282828, 0xFF2B2B2B, 0xFF2E2E2E,
                0xFF313131, 0xFF323232, 0xFF323232, 0xFF323232, 0xFF313131, 0xFF2E2E2E,
                0xFF2B2B2B, 0xFF282828, 0xFF242424, 0xFF1F1F1F, 0xFF1B1B1B, 0xFF161616,
                0xFF121212, 0xFF0D0D0D, 0xFF090909, 0xFF050505, 0xFF020202, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF010101, 0xFF040404, 0xFF070707, 0xFF0B0B0B,
                0xFF0E0E0E, 0xFF131313, 0xFF171717, 0xFF1B1B1B, 0xFF1F1F1F, 0xFF232323,
                0xFF262626, 0xFF292929, 0xFF2B2B2B, 0xFF2C2C2C, 0xFF2C2C2C, 0xFF2C2C2C,
                0xFF2B2B2B, 0xFF292929, 0xFF262626, 0xFF232323, 0xFF1F1F1F, 0xFF1B1B1B,
                0xFF171717, 0xFF131313, 0xFF0E0E0E, 0xFF0B0B0B, 0xFF070707, 0xFF040404,
                0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF020202,
                0xFF050505, 0xFF080808, 0xFF0C0C0C, 0xFF0F0F0F, 0xFF131313, 0xFF171717,
                0xFF1B1B1B, 0xFF1E1E1E, 0xFF212121, 0xFF232323, 0xFF252525, 0xFF262626,
                0xFF272727, 0xFF262626, 0xFF252525, 0xFF232323, 0xFF212121, 0xFF1E1E1E,
                0xFF1B1B1B, 0xFF171717, 0xFF131313, 0xFF0F0F0F, 0xFF0C0C0C, 0xFF080808,
                0xFF050505, 0xFF020202, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF010101, 0xFF030303, 0xFF060606, 0xFF090909, 0xFF0C0C0C,
                0xFF0F0F0F, 0xFF131313, 0xFF161616, 0xFF191919, 0xFF1B1B1B, 0xFF1E1E1E,
                0xFF1F1F1F, 0xFF202020, 0xFF212121, 0xFF202020, 0xFF1F1F1F, 0xFF1E1E1E,
                0xFF1B1B1B, 0xFF191919, 0xFF161616, 0xFF131313, 0xFF0F0F0F, 0xFF0C0C0C,
                0xFF090909, 0xFF060606, 0xFF030303, 0xFF010101, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF030303,
                0xFF060606, 0xFF090909, 0xFF0C0C0C, 0xFF0E0E0E, 0xFF121212, 0xFF141414,
                0xFF161616, 0xFF181818, 0xFF191919, 0xFF1A1A1A, 0xFF1B1B1B, 0xFF1A1A1A,
                0xFF191919, 0xFF181818, 0xFF161616, 0xFF141414, 0xFF121212, 0xFF0E0E0E,
                0xFF0C0C0C, 0xFF090909, 0xFF060606, 0xFF030303, 0xFF010101, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF010101, 0xFF030303, 0xFF060606, 0xFF080808, 0xFF0B0B0B,
                0xFF0D0D0D, 0xFF0F0F0F, 0xFF111111, 0xFF131313, 0xFF141414, 0xFF151515,
                0xFF151515, 0xFF151515, 0xFF141414, 0xFF131313, 0xFF111111, 0xFF0F0F0F,
                0xFF0D0D0D, 0xFF0B0B0B, 0xFF080808, 0xFF060606, 0xFF030303, 0xFF010101,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF030303,
                0xFF050505, 0xFF070707, 0xFF090909, 0xFF0B0B0B, 0xFF0C0C0C, 0xFF0E0E0E,
                0xFF0F0F0F, 0xFF0F0F0F, 0xFF101010, 0xFF0F0F0F, 0xFF0F0F0F, 0xFF0E0E0E,
                0xFF0C0C0C, 0xFF0B0B0B, 0xFF090909, 0xFF070707, 0xFF050505, 0xFF030303,
                0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF010101, 0xFF020202, 0xFF040404, 0xFF050505, 0xFF070707,
                0xFF080808, 0xFF090909, 0xFF0A0A0A, 0xFF0B0B0B, 0xFF0B0B0B, 0xFF0B0B0B,
                0xFF0A0A0A, 0xFF090909, 0xFF080808, 0xFF070707, 0xFF050505, 0xFF040404,
                0xFF020202, 0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101,
                0xFF020202, 0xFF030303, 0xFF040404, 0xFF050505, 0xFF060606, 0xFF060606,
                0xFF060606, 0xFF060606, 0xFF060606, 0xFF050505, 0xFF040404, 0xFF030303,
                0xFF020202, 0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF010101, 0xFF010101, 0xFF020202,
                0xFF020202, 0xFF030303, 0xFF030303, 0xFF030303, 0xFF020202, 0xFF020202,
                0xFF010101, 0xFF010101, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
                0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000};

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT,
                     0, GL_BGRA, GL_UNSIGNED_BYTE, image_array + 8);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        TODO glTexEnvf
//        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    }

    return textureID;
}

void clearScreen () {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void updateWindow() {
    SDL_GL_SwapWindow(window);
}