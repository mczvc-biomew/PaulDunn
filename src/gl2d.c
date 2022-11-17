/**
	Easy GL2D 1.1

	[Relminator] (2010)
	Richard Eric M. Lope BSN RN

	http://rel.betterwebber.com

	[mczvc-biomew] (2022)
	Meldencio Czarlemagne Veras Corrales, CS

    https://github.com/mczvc827
*/


#include "gl2d.h"
//#include <glad/glad.h>
#include <SDL2/SDL_image.h>
#include <GL/glut.h>

// MingW header excluding GL_BGRA in windows
#ifndef GL_BGRA
#define GL_BGRA  0x80E1
#endif

static GLuint gCurrentTexture = 0;

static SDL_Surface *screenSurface = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;


//    Private Functions


#define MAX_BATCH_DRAW 256
static GLfloat vertexes[MAX_BATCH_DRAW * 2 * 6];
static GLfloat textures[MAX_BATCH_DRAW * 2 * 6];
static GLfloat colors[MAX_BATCH_DRAW * 4 * 6 * 2];
static int vertexIndex;
static int textureIndex;
static int colorIndex;

// OpenGL ES 2.0 uses shaders
static const char *VERTEX_SHADER = "#version 330 core\n"
                            "attribute vec4 a_position;\n"
                            "attribute vec4 a_color;\n"
                            "attribute vec2 a_texCoord0;\n"
                            "varying vec4 v_color;\n"
                            "varying vec2 v_texCoords;"
                            "void main()\n"
                            "{\n"
                            "gl_Position = vec4(a_position.xyz, 1.0);\n"
                            "gl_PointSize = 1.0;\n"
                            "v_color = a_color;\n"
                            "v_texCoords = a_texCoord0;\n"
                            "}";

static const char *FRAGMENT_SHADER = "#version 330 core\n"
                              "precision mediump float;\n"
                              "varying vec4 v_color;\n"
                              "varying vec2 v_texCoords;\n"
                              "uniform sampler2D u_texture;\n"
                              "void main()\n"
                              "{\n"
                              "gl_FragColor = v_color + texture2D(u_texture, v_texCoords);\n"
                              "}";


static void init_renderer(int originX, int originY, int width, int height) {
//    double FOVy;
//    double aspect;
//    double znear;
//    double zfar;

    glViewport(originX, originY, width, height);

// Load shaders
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &VERTEX_SHADER, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &FRAGMENT_SHADER, NULL);
    glCompileShader(fragment);

    // Combine shaders into program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    // Use it
    glUseProgram(program);

    // Point position attribute to vertexes
    GLint position = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertexes);
    // Point texel attribute to vertexes
    GLint texture = glGetAttribLocation(program, "a_texCoord0");
    glEnableVertexAttribArray(texture);
    glVertexAttribPointer(texture, 2, GL_FLOAT, GL_FALSE, 0, textures);
    // Point color attribute to colors
    GLint color = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, 0, colors);

    gCurrentTexture = 0;
    vertexIndex = 0;
    colorIndex = 0;
}

static void init2D() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    // Set gluPerspective params
//    FOVy = 90 / 2; //45 deg fovy
//    aspect = width / height;
//    znear = 1;     //Near clip
//    zfar = 500;    //far clip
//
//    // use glu Perspective to set our 3d frustum dimension up
//    gluPerspective(FOVy, aspect, znear, zfar);

    // Modelview mode
    // ie. Matrix that does things to anything we draw
    // as in lines, points, tris, etc.
//    glMatrixMode(GL_MODELVIEW);

    // load identity(clean) matrix to modelview
//    glLoadIdentity();

    glClearDepth(1.0);                       //'Set Depth buffer to 1(z-Buffer)
    glDisable(GL_DEPTH_TEST);                //'Disable Depth Testing so that our z-buffer works

    //'compare each incoming pixel z value with the z value present in the depth buffer
    //'LEQUAL means than pixel is drawn if the incoming z value is less than
    //'or equal to the stored z value
    glDepthFunc(GL_LEQUAL);

//    'have one or more material parameters track the current color
//    'Material is your 3d model
//    glEnable(GL_COLOR_MATERIAL);

    //Enable Texturing
    glEnable(GL_TEXTURE_2D);


//    'Tell openGL that we want the best possible perspective transform
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glPolygonMode(GL_FRONT, GL_FILL);

//    glEnable(GL_ALPHA_TEST);
//    glAlphaFunc(GL_GREATER, 0);
    //'Disable Backface culling
    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
//    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
//    glDisable(GL_FOG);

//    glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//    glOrtho(0, width, height, 0, -1, 1);
//    glMatrixMode(GL_MODELVIEW);
//    glPushMatrix();
//    glLoadIdentity();
//    glTranslatef(0.375, 0.375, 0.0);

}
/**
 * Creates new Window. Defaults to SDL Window
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

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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
    init_renderer(g_cropH, g_cropV, g_scaledWidth, g_scaledHeight);

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

void GL2D_Quit() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}
/******************************************************************************

   Private!!!

******************************************************************************/

static SDL_Surface *LoadTexture(const char *filename) {
    SDL_Surface *img = IMG_Load(filename);

    if (img == NULL) {
        fprintf(stderr, "Could not load image: %s; SDL:: %s\n", filename, SDL_GetError());
        return NULL;
    }

    SDL_SetSurfaceBlendMode(img, SDL_BLENDMODE_BLEND);
    SDL_Surface *optimizedImg = SDL_ConvertSurface(img, screenSurface->format, 0);
    if (optimizedImg == NULL) fprintf(stderr, "could not optimize image: %s\n", SDL_GetError());
    SDL_FreeSurface(img);

    SDL_SetColorKey(optimizedImg, SDL_TRUE, SDL_MapRGB(optimizedImg->format, 0, 0, 0));

    SDL_SetSurfaceBlendMode(optimizedImg, SDL_BLENDMODE_BLEND);

    return optimizedImg;

}

/******************************************************************************


******************************************************************************/

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

/**
 TODO blending is weird in SDL2. Maybe fix it soon
*/

void glBlendMode(const GL2D_BLEND_MODE mode) {
    switch (mode) {
        case GL2D_TRANS:
            glEnable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            //glBlendFunc( GL_ONE, GL_ONE );
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            break;

        case GL2D_SOLID:
            glDisable(GL_BLEND);
//            glDisable(GL_ALPHA_TEST);

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            break;

        case GL2D_BLENDED:
            glEnable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            //glBlendFunc( GL_SRC_ALPHA, GL_ONE );

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            break;

        case GL2D_GLOW:
            glEnable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            break;

        case GL2D_ALPHA:
            glEnable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            break;

        case GL2D_BLACK:
            glEnable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            //glBlendFunc( GL_ZERO,GL_ONE_MINUS_SRC_ALPHA );
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
            break;

        default:
            glDisable(GL_BLEND);
            //glEnable( GL_ALPHA_TEST );
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_MOD);
    }

}

//
///**
// * Clear the screen */
void clearScreen () {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
//
void glLineGlow(const double x1, const double y1,
                const double x2, const double y2,
                const double width, const GLfloat r,
                const GLfloat g, const GLfloat b,
                const GLfloat a
) {

    GLuint textureID;

    textureID = GetGlowImage();
//    TODO glColor4ub
//    glColor4ubv((GLubyte *) (&color));

    // Only change active texture when there is a need
    // Speeds up the rendering by batching textures

    if (textureID != gCurrentTexture) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        gCurrentTexture = textureID;
    }


    float nx, ny;
    nx = -(y2 - y1);
    ny = (x2 - x1);

    float leng;
    leng = sqrt(nx * nx + ny * ny);
    nx = nx / leng;
    ny = ny / leng;

    nx *= width / 2.0;
    ny *= width / 2.0;

    float lx1, ly1, lx2, ly2, lx3, ly3, lx4, ly4;

    lx1 = x2 + nx;
    ly1 = y2 + ny;
    lx2 = x2 - nx;
    ly2 = y2 - ny;
    lx3 = x1 - nx;
    ly3 = y1 - ny;
    lx4 = x1 + nx;
    ly4 = y1 + ny;

    // MAIN
    if (vertexIndex + 12 > MAX_BATCH_DRAW*2*6 ||
        colorIndex + 4*6 > MAX_BATCH_DRAW*4*6*2 ||
        textureIndex + 12 > MAX_BATCH_DRAW*2*6) {
        flush();
    }
    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 0] = lx1;
    vertexes[vertexIndex + 1] = ly1;
    textures[textureIndex + 0] = 0.5f;
    textures[textureIndex + 1] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 2] = lx2;
    vertexes[vertexIndex + 3] = ly2;
    textures[textureIndex + 2] = 0.5f;
    textures[textureIndex + 3] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 4] = lx3;
    vertexes[vertexIndex + 5] = ly3;
    textures[textureIndex + 4] = 0.5f;
    textures[textureIndex + 5] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 6] = lx1;
    vertexes[vertexIndex + 7] = ly1;
    textures[textureIndex + 6] = 0.5f;
    textures[textureIndex + 7] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 8] = lx3;
    vertexes[vertexIndex + 9] = ly3;
    textures[textureIndex + 8] = 0.5f;
    textures[textureIndex + 9] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 10] = lx4;
    vertexes[vertexIndex + 11] = ly4;
    textures[textureIndex + 10] = 0.5f;
    textures[textureIndex + 11] = 0.0f;

    vertexIndex += 12;
    textureIndex += 12;
//    glBegin(GL_TRIANGLES);
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx1, ly1, 0.0);
//    glTexCoord2f(0.5, 1.0);
//    glVertex3f(lx2, ly2, 0.0);
//    glTexCoord2f(0.5, 1.0);
//    glVertex3f(lx3, ly3, 0.0);
//
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx1, ly1, 0.0);
//    glTexCoord2f(0.5, 1.0);
//    glVertex3f(lx3, ly3, 0.0);
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx4, ly4, 0.0);
//    glEnd();

    //RIGHT
    float lx5, ly5, lx6, ly6, vx, vy;
    vx = (x2 - x1);
    vy = (y2 - y1);
    leng = sqrt(vx * vx + vy * vy);
    vx = vx / leng;
    vy = vy / leng;
    vx *= width / 2.0;
    vy *= width / 2.0;

    lx5 = lx1 + vx;
    ly5 = ly1 + vy;
    lx6 = lx2 + vx;
    ly6 = ly2 + vy;

//    glBegin(GL_TRIANGLES);
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx1, ly1, 0.0);
//    glTexCoord2f(1.0, 0.0);
//    glVertex3f(lx5, ly5, 0.0);
//    glTexCoord2f(1.0, 1.0);
//    glVertex3f(lx6, ly6, 0.0);
//
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx1, ly1, 0.0);
//    glTexCoord2f(1.0, 1.0);
//    glVertex3f(lx6, ly6, 0.0);
//    glTexCoord2f(0.5, 1.0);
//    glVertex3f(lx2, ly2, 0.0);
//    glEnd();
    if (vertexIndex + 12 > MAX_BATCH_DRAW*2*6 ||
        colorIndex + 4*6 > MAX_BATCH_DRAW*4*6*2 ||
        textureIndex + 12 > MAX_BATCH_DRAW*2*6) {
        flush();
    }
    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 0] = lx1;
    vertexes[vertexIndex + 1] = ly1;
    textures[textureIndex + 0] = 0.5f;
    textures[textureIndex + 1] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 2] = lx5;
    vertexes[vertexIndex + 3] = ly5;
    textures[textureIndex + 2] = 1.0f;
    textures[textureIndex + 3] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 4] = lx6;
    vertexes[vertexIndex + 5] = ly6;
    textures[textureIndex + 4] = 1.0f;
    textures[textureIndex + 5] = 1.0f;

//    colors[colorIndex + 0] = r;
//    colors[colorIndex + 1] = g;
//    colors[colorIndex + 2] = b;
//    colors[colorIndex + 3] = a;
//    colorIndex += 4;

    vertexes[vertexIndex + 6] = lx1;
    vertexes[vertexIndex + 7] = ly1;
    textures[textureIndex + 6] = 0.5f;
    textures[textureIndex + 7] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 8] = lx6;
    vertexes[vertexIndex + 9] = ly6;
    textures[textureIndex + 8] = 1.0f;
    textures[textureIndex + 9] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 10] = lx2;
    vertexes[vertexIndex + 11] = ly2;
    textures[textureIndex + 10] = 0.5f;
    textures[textureIndex + 11] = 1.0f;

    vertexIndex += 12;
    textureIndex += 12;

    // LEFT
    lx5 = lx4 - vx;
    ly5 = ly4 - vy;
    lx6 = lx3 - vx;
    ly6 = ly3 - vy;

//    glBegin(GL_TRIANGLES);
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx4, ly4, 0.0);
//    glTexCoord2f(0.5, 1.0);
//    glVertex3f(lx3, ly3, 0.0);
//    glTexCoord2f(1.0, 1.0);
//    glVertex3f(lx6, ly6, 0.0);
//
//    glTexCoord2f(0.5, 0.0);
//    glVertex3f(lx4, ly4, 0.0);
//    glTexCoord2f(1.0, 1.0);
//    glVertex3f(lx6, ly6, 0.0);
//    glTexCoord2f(1.0, 0.0);
//    glVertex3f(lx5, ly5, 0.0);
//    glEnd();
    if (vertexIndex + 12 > MAX_BATCH_DRAW*2*6 ||
        colorIndex + 4*6 > MAX_BATCH_DRAW*4*6*2 ||
        textureIndex + 12 > MAX_BATCH_DRAW*2*6) {
        flush();
    }
    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 0] = lx4;
    vertexes[vertexIndex + 1] = ly4;
    textures[textureIndex + 0] = 0.5f;
    textures[textureIndex + 1] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 2] = lx3;
    vertexes[vertexIndex + 3] = ly3;
    textures[textureIndex + 2] = 0.5f;
    textures[textureIndex + 3] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 4] = lx6;
    vertexes[vertexIndex + 5] = ly6;
    textures[textureIndex + 4] = 1.0f;
    textures[textureIndex + 5] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 6] = lx4;
    vertexes[vertexIndex + 7] = ly4;
    textures[textureIndex + 6] = 0.5f;
    textures[textureIndex + 7] = 0.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 8] = lx6;
    vertexes[vertexIndex + 9] = ly6;
    textures[textureIndex + 8] = 1.0f;
    textures[textureIndex + 9] = 1.0f;

    colors[colorIndex + 0] = r;
    colors[colorIndex + 1] = g;
    colors[colorIndex + 2] = b;
    colors[colorIndex + 3] = a;
    colorIndex += 4;

    vertexes[vertexIndex + 10] = lx5;
    vertexes[vertexIndex + 11] = ly5;
    textures[textureIndex + 10] = 1.0f;
    textures[textureIndex + 11] = 0.0f;

    vertexIndex += 12;
    textureIndex += 12;

//    if (vertexIndex >= (MAX_BATCH_DRAW*2*6 - 12*(MAX_BATCH_DRAW-3)))
//        flush();

//    glColor4ub(255, 255, 255, 255);

}

void flush() {
    glDrawArrays(GL_TRIANGLES, 0, vertexIndex/(2*2*3));
    vertexIndex = 0;
    colorIndex = 0;
    textureIndex = 0;
}


///******************************************************************************
//
//
//******************************************************************************/
//void glPutPixel(int x1, int y1, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    glBegin(GL_POINTS);
//    glVertex2i(x1, y1);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///******************************************************************************
//
//
//******************************************************************************/
//void glLine(int x1, int y1, int x2, int y2, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    glBegin(GL_LINES);
//    glVertex2i(x1, y1);
//    glVertex2i(x2, y2);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///******************************************************************************
//
//
//******************************************************************************/
//void glLineGradient(int x1, int y1, int x2, int y2,
//                    const GLuint color1, const GLuint color2
//) {
//    glDisable(GL_TEXTURE_2D);
//
//    glBegin(GL_LINES);
//    glColor4ubv((GLubyte *) (&color1));
//    glVertex2i(x1, y1);
//    glColor4ubv((GLubyte *) (&color2));
//    glVertex2i(x2, y2);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///******************************************************************************
//
//
//******************************************************************************/
//void glBox(int x1, int y1, int x2, int y2, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    glBegin(GL_LINE_STRIP);
//    glVertex2i(x1, y1);
//    glVertex2i(x2, y1);
//    glVertex2i(x2, y2);
//    glVertex2i(x1, y2);
//    glVertex2i(x1, y1);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///******************************************************************************
//
//
//******************************************************************************/
//
//void glBoxFilled(int x1, int y1, int x2, int y2, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    x2++;
//    y2++;
//
//    glBegin(GL_TRIANGLES);
//
//    glVertex2i(x1, y1);
//    glVertex2i(x1, y2);
//    glVertex2i(x2, y2);
//
//    glVertex2i(x1, y1);
//    glVertex2i(x2, y2);
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
//
///******************************************************************************
//
//
//******************************************************************************/
//
//void glBoxFilledGradient(int x1, int y1, int x2, int y2,
//                         const GLuint color1, const GLuint color2,
//                         const GLuint color3, const GLuint color4
//) {
//    glDisable(GL_TEXTURE_2D);
//
//    x2++;
//    y2++;
//
//    glBegin(GL_TRIANGLES);
//
//    glColor4ubv((GLubyte *) (&color1));
//    glVertex2i(x1, y1);
//    glColor4ubv((GLubyte *) (&color2));
//    glVertex2i(x1, y2);
//    glColor4ubv((GLubyte *) (&color3));
//    glVertex2i(x2, y2);
//
//    glColor4ubv((GLubyte *) (&color1));
//    glVertex2i(x1, y1);
//    glColor4ubv((GLubyte *) (&color3));
//    glVertex2i(x2, y2);
//    glColor4ubv((GLubyte *) (&color4));
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
//
///******************************************************************************
//
//
//******************************************************************************/
//void glTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    glBegin(GL_LINE_STRIP);
//    glVertex2i(x1, y1);
//    glVertex2i(x2, y2);
//    glVertex2i(x3, y3);
//    glVertex2i(x1, y1);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///******************************************************************************
//
//
//******************************************************************************/
//void glTriangleFilled(int x1, int y1, int x2, int y2, int x3, int y3, const GLuint color) {
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    glBegin(GL_TRIANGLES);
//    glVertex2i(x1, y1);
//    glVertex2i(x2, y2);
//    glVertex2i(x3, y3);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
//
///******************************************************************************
//
//
//******************************************************************************/
//void glTriangleFilledGradient(int x1, int y1, int x2, int y2, int x3, int y3,
//                              const GLuint color1, const GLuint color2, const GLuint color3
//) {
//    glDisable(GL_TEXTURE_2D);
//
//    glBegin(GL_TRIANGLES);
//    glColor4ubv((GLubyte *) (&color1));
//    glVertex2i(x1, y1);
//    glColor4ubv((GLubyte *) (&color2));
//    glVertex2i(x2, y2);
//    glColor4ubv((GLubyte *) (&color3));
//    glVertex2i(x3, y3);
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///**
//
//    Contributed by Michael "h4tt3n" Nissen(I added a filled version ;*)) \n
//    syntax: ellipse(center x, center y, semi-major axis, semi-minor axis, angle in degrees, color)
//
//*/
//void glEllipse(int x, int y, int a, int b, int degrees, const GLuint color) {
//
//    // these constants decide the quality of the ellipse
//    const float pi = M_PI;
//    const float twopi = 2 * pi;   //        two pi (radians in a circle)
//    const int face_length = 8;     //        approx. face length in pixels
//    const int max_faces = 256;      //        maximum number of faces in ellipse
//    const int min_faces = 16;       //        minimum number of faces in ellipse
//
//    // approx. ellipse circumference (hudson's method)
//    float h = (a - b * a - b) / (float) (a + b * a + b);
//    float circumference = 0.25 * pi * (a + b) * (3 * (1 + h * 0.25) + 1 / (float) (1 - h * 0.25));
//
//    // number of faces in ellipse
//    int num_faces = circumference / face_length;
//
//    // clamp number of faces
//    if (num_faces > max_faces) num_faces = max_faces;
//    if (num_faces < min_faces) num_faces = min_faces;
//
//    // keep number of faces divisible by 4
//    num_faces -= (num_faces & 3);
//
//    // precalc cosine theta
//    float angle = degrees * M_PI / 180.0;
//    float s = sin(twopi / (float) num_faces);
//    float c = cos(twopi / (float) num_faces);
//    float xx = 1;
//    float yy = 0;
//    float xt = 0;
//    float ax = cos(angle);
//    float ay = sin(angle);
//
//
//    // draw ellipse
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    int i;
//    glBegin(GL_LINE_LOOP);
//
//    for (i = 0; i < num_faces; i++) {
//        xt = xx;
//        xx = c * xx - s * yy;
//        yy = s * xt + c * yy;
//        glVertex2f(x + a * xx * ax - b * yy * ay, y + a * xx * ay + b * yy * ax);
//    }
//
//    glVertex2f(x + a * ax, y + a * ay);
//
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///**
//
//    Contributed by Michael "h4tt3n" Nissen(I added a filled version ;*)) \n
//    syntax: ellipse(center x, center y, semimajor axis, semiminor axis, angle in degrees, color)
//
//*/
//void glEllipseFilled(int x, int y, int a, int b, int degrees, const GLuint color) {
//
//    // these constants decide the quality of the ellipse
//    const float pi = M_PI;
//    const float twopi = 2 * pi;   //        two pi (radians in a circle)
//    const int face_length = 8;     //        approx. face length in pixels
//    const int max_faces = 256;      //        maximum number of faces in ellipse
//    const int min_faces = 16;       //        minimum number of faces in ellipse
//
//    // approx. ellipse circumference (hudson's method)
//    float h = (a - b * a - b) / (float) (a + b * a + b);
//    float circumference = 0.25 * pi * (a + b) * (3 * (1 + h * 0.25) + 1 / (float) (1 - h * 0.25));
//
//    // number of faces in ellipse
//    int num_faces = circumference / face_length;
//
//    // clamp number of faces
//    if (num_faces > max_faces) num_faces = max_faces;
//    if (num_faces < min_faces) num_faces = min_faces;
//
//    // keep number of faces divisible by 4
//    num_faces -= (num_faces & 3);
//
//    // precalc cosine theta
//    float angle = degrees * M_PI / 180.0;
//    float s = sin(twopi / (float) num_faces);
//    float c = cos(twopi / (float) num_faces);
//    float xx = 1;
//    float yy = 0;
//    float xt = 0;
//    float ax = cos(angle);
//    float ay = sin(angle);
//
//
//    // draw ellipse
//    glDisable(GL_TEXTURE_2D);
//    glColor4ubv((GLubyte *) (&color));
//
//    int i;
//    glBegin(GL_TRIANGLE_FAN);
//
//    for (i = 0; i < num_faces; i++) {
//        xt = xx;
//        xx = c * xx - s * yy;
//        yy = s * xt + c * yy;
//        glVertex2f(x + a * xx * ax - b * yy * ay, y + a * xx * ay + b * yy * ax);
//    }
//
//    glVertex2f(x + a * ax, y + a * ay);
//
//    glEnd();
//
//    glEnable(GL_TEXTURE_2D);
//
//    glColor4ub(255, 255, 255, 255);
//
//}
//
///**
//
//    Draws a sprite. \n
//	Parameters: \n
//       x 		-> x position of the sprite \n
//	   y 		-> y position of the sprite \n
//	   flipmode -> mode for flipping (see GL_FLIP_MODE enum) \n
//	   *spr 	-> pointer to a glImage \n
//
//*/
//void glSprite(const int x, const int y, const int flipmode, const glImage *spr) {
//    int x1 = x;
//    int y1 = y;
//    int x2 = x + spr->width;
//    int y2 = y + spr->height;
//
//    float u1 = (flipmode & GL2D_FLIP_H) ? spr->u_off + spr->u_width : spr->u_off;
//    float u2 = (flipmode & GL2D_FLIP_H) ? spr->u_off : spr->u_off + spr->u_width;
//    float v1 = (flipmode & GL2D_FLIP_V) ? spr->v_off + spr->v_height : spr->v_off;
//    float v2 = (flipmode & GL2D_FLIP_V) ? spr->v_off : spr->v_off + spr->v_height;
//
//
//    if (spr->textureID != gCurrentTexture) {
//        glBindTexture(GL_TEXTURE_2D, spr->textureID);
//        gCurrentTexture = spr->textureID;
//    }
//
//    glBegin(GL_TRIANGLES);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u1, v2);
//    glVertex2i(x1, y2);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//    glTexCoord2f(u2, v1);
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//
//}
//
///**
//
//    Draws a scaled sprite
//	Parameters:
//       x 		-> x position of the sprite
//	   y 		-> y position of the sprite
//	   scale    -> float value to scale the sprite (1.0 for no scaling)
//	   flipmode -> mode for flipping (see GL_FLIP_MODE enum)
//	   *spr 	-> pointer to a glImage
//
//*/
//void glSpriteScale(const int x, const int y, const float scale, const int flipmode, const glImage *spr) {
//    int x1 = 0;
//    int y1 = 0;
//    int x2 = spr->width;
//    int y2 = spr->height;
//
//    float u1 = (flipmode & GL2D_FLIP_H) ? spr->u_off + spr->u_width : spr->u_off;
//    float u2 = (flipmode & GL2D_FLIP_H) ? spr->u_off : spr->u_off + spr->u_width;
//    float v1 = (flipmode & GL2D_FLIP_V) ? spr->v_off + spr->v_height : spr->v_off;
//    float v2 = (flipmode & GL2D_FLIP_V) ? spr->v_off : spr->v_off + spr->v_height;
//
//
//    if (spr->textureID != gCurrentTexture) {
//        glBindTexture(GL_TEXTURE_2D, spr->textureID);
//        gCurrentTexture = spr->textureID;
//    }
//
//    glPushMatrix();
//
//    glTranslatef(x, y, 0.0);
//    glScalef(scale, scale, 1.0);
//
//    glBegin(GL_TRIANGLES);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u1, v2);
//    glVertex2i(x1, y2);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//    glTexCoord2f(u2, v1);
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//    glPopMatrix();
//
//}
//
///**
//
//    Draws a center rotated sprite
//	Parameters:
//       x 		-> x position of the sprite center
//	   y 		-> y position of the sprite center
//	   angle    -> is the angle to rotated (in degrees)
//	   flipmode -> mode for flipping (see GL_FLIP_MODE enum)
//	   *spr 	-> pointer to a glImage
//
//*/
//void glSpriteRotate(const int x, const int y, const int angle, const int flipmode, const glImage *spr) {
//
//    int s_half_x = spr->width / 2;
//    int s_half_y = spr->height / 2;
//
//    int x1 = -s_half_x;
//    int y1 = -s_half_y;
//
//    int x2 = s_half_x;
//    int y2 = s_half_y;
//
//
//    float u1 = (flipmode & GL2D_FLIP_H) ? spr->u_off + spr->u_width : spr->u_off;
//    float u2 = (flipmode & GL2D_FLIP_H) ? spr->u_off : spr->u_off + spr->u_width;
//    float v1 = (flipmode & GL2D_FLIP_V) ? spr->v_off + spr->v_height : spr->v_off;
//    float v2 = (flipmode & GL2D_FLIP_V) ? spr->v_off : spr->v_off + spr->v_height;
//
//
//    if (spr->textureID != gCurrentTexture) {
//        glBindTexture(GL_TEXTURE_2D, spr->textureID);
//        gCurrentTexture = spr->textureID;
//    }
//
//    glPushMatrix();
//
//    glTranslatef(x, y, 0.0);
//    glRotatef(angle, 0.0, 0.0, 1.0);
//
//
//    glBegin(GL_TRIANGLES);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u1, v2);
//    glVertex2i(x1, y2);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//    glTexCoord2f(u2, v1);
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//    glPopMatrix();
//
//
//}
//
//
///**
//
//    Draws a center rotated and scaled sprite
//	Parameters:
//       x 		-> x position of the sprite center
//	   y 		-> y position of the sprite center
//	   angle    -> is the angle to rotated (-32768 to 32767)
//	   scale    -> is a 20.12 FP scale value (you can use floattof32(1.45) to scale
//	               and GCC would optimize it to a constant)
//	   flipmode -> mode for flipping (see GL_FLIP_MODE enum)
//	   *spr 	-> pointer to a glImage
//
//*/
//void glSpriteRotateScale(const int x, const int y, const int angle, const float scale, const int flipmode,
//                         const glImage *spr) {
//
//    int s_half_x = spr->width / 2;
//    int s_half_y = spr->height / 2;
//
//    int x1 = -s_half_x;
//    int y1 = -s_half_y;
//
//    int x2 = s_half_x;
//    int y2 = s_half_y;
//
//
//    float u1 = (flipmode & GL2D_FLIP_H) ? spr->u_off + spr->u_width : spr->u_off;
//    float u2 = (flipmode & GL2D_FLIP_H) ? spr->u_off : spr->u_off + spr->u_width;
//    float v1 = (flipmode & GL2D_FLIP_V) ? spr->v_off + spr->v_height : spr->v_off;
//    float v2 = (flipmode & GL2D_FLIP_V) ? spr->v_off : spr->v_off + spr->v_height;
//
//
//    if (spr->textureID != gCurrentTexture) {
//        glBindTexture(GL_TEXTURE_2D, spr->textureID);
//        gCurrentTexture = spr->textureID;
//    }
//
//    glPushMatrix();
//
//    glTranslatef(x, y, 0.0);
//    glScalef(scale, scale, 1.0);
//    glRotatef(angle, 0.0, 0.0, 1.0);
//
//    glBegin(GL_TRIANGLES);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u1, v2);
//    glVertex2i(x1, y2);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//
//    glTexCoord2f(u1, v1);
//    glVertex2i(x1, y1);
//    glTexCoord2f(u2, v2);
//    glVertex2i(x2, y2);
//    glTexCoord2f(u2, v1);
//    glVertex2i(x2, y1);
//
//    glEnd();
//
//    glPopMatrix();
//
//
//}

/**

    Initializes our spriteset with Texture Packer generated UV coordinates
	Very safe and easy to use.
	Parameters:
       filename     -> name of the file to load as texture
       *sprite 		-> pointer to an array of glImage
       width/height -> image dimensions
	   numframes 	-> number of frames in a spriteset (auto-generated by Texture Packer)
	   *texcoords 	-> Texture Packer auto-generated array of UV coords
	   textureID 	-> Texture handle returned by glGenTextures()
       filtemode    -> GL_NEAREST or GL_LINEAR

*/
GLuint glLoadSpriteset(const char *filename,
                       glImage *sprite,
                       const int width,
                       const int height,
                       const int numframes,
                       const unsigned int *texcoords,
                       const GLuint filtermode
) {

    int i;
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_Surface *img = LoadTexture(filename);
    if (img == NULL) return 0;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermode);
    // TODO glTexEnvf
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->pixels);

    SDL_FreeSurface(img);
    glBindTexture(GL_TEXTURE_2D, 0);

    // init sprites texture coords and texture ID
    for (i = 0; i < numframes; i++) {
        int j = i * 4; // texcoords array is u_off, wid, hei
        sprite[i].textureID = textureID;
        sprite[i].width = texcoords[j + 2];
        sprite[i].height = texcoords[j + 3];
        sprite[i].u_off = texcoords[j] / (float) width;                // set x-coord
        sprite[i].v_off = texcoords[j + 1] / (float) height;            // y-coord
        sprite[i].u_width = texcoords[j + 2] / (float) width;            // set x-coord
        sprite[i].v_height = texcoords[j + 3] / (float) height;        // y-coord

    }

    return textureID;

}


GLuint glLoadSprite(const char *filename,
                    glImage *sprite,
                    const int width,
                    const int height,
                    const GLuint filterMode
) {

    GLuint textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_Surface *img = LoadTexture(filename);
    if (img == NULL) return 0;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    // TODO glTexEnvf
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->pixels);

    SDL_FreeSurface(img);
    glBindTexture(GL_TEXTURE_2D, 0);

    sprite->textureID = textureID;
    sprite->width = width;
    sprite->height = height;
    sprite->u_off = 0.0;
    sprite->v_off = 0.0;
    sprite->u_width = 1.0;
    sprite->v_height = 1.0;


    return textureID;

}

void updateWindow() {
    //SDL_UpdateWindowSurface(window);
    //SDL_RenderPresent(renderer);
    SDL_GL_SwapWindow(window);
}
