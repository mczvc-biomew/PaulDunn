#ifndef EGG_H_INCLUDED
/**
 * \EGG2D Egg2D Library.
 * A small 2D capable, 3D graphics library.
 *
 * @file egg2d.h\n
 * Easy Game Graphics v0.3.5
 *
 * (c) 2022-2025 [mcvzc][<a href="mailto:mczvc\@proton.me">email me</a>]<br><br>
 * <b>Meldencio Czarlemagne Corrales</b>,
 *
 * <li> <a href="https://github.com/mczvc-biomew">Github</a> </li>
 * <li> <a href="https://mczvc-biomew.github.io/mczvc">Github Personal Page</a> </li>
 */
#define EGG_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#if defined(__ANDROID__) && defined(GLES2)
  #include <GLES2/gl2.h>
#elif defined(__ANDROID__) && defined(GLES1)
  #include <GLES/gl.h>
#else
  #include <glad/glad.h>
#endif
#ifdef __cplusplus

extern "C" {
#endif

//  Macros

#ifdef ANDROID
#include <android/log.h>
#include <android/asset_manager.h>
typedef AAsset eggFile;
#else
typedef FILE eggFile;
#endif

#ifdef WIN32
#define EGG_API __cdecl
#else
#define EGG_API
#endif

#define GL_CHECK(x) \
    x;              \
    {               \
        GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR) {   \
            SDL_Log("glGetError() = %i (0x%.8x) at line %i\n", glError, glError, __LINE__); \
        }           \
    }

EGG_API struct EggFileContext {
    eggFile *filePointer;
    long size;
};
typedef struct EggFileContext EggFileContext;
// --- Shader Manager ---
struct TextResource {
    const char *src;
    long size;
};

#define SHADER_VERT 0
#define SHADER_FRAG 1
#define SHADER_NO_ERROR 0
#define SHADER_READ_ERROR (-1)
#define SHADER_COMPILE_ERROR (-2)

struct EggShader {
    int type;
    GLuint id;
    struct TextResource src;
    int error;
};

typedef struct EggShader EggShader;

EGG_API struct EggFileContext eggFileOpen(void *ioContext, const char *fileName);
EGG_API void eggFileClose(eggFile *pFile);
EGG_API size_t eggFileRead(eggFile *pFile, long bytesToRead, void *buffer);

struct EggShader eggLoadVertShaderFile(const char *relativePath);
struct EggShader eggLoadFragShaderFile(const char *relativePath);
void eggFreeShader(EggShader obj);

extern int g_targetWidth, g_targetHeight;
extern double g_targetAspect;

static int g_actualWidth, g_actualHeight;
static int g_aspect;
static float g_scale;
static int g_scaledWidth, g_scaledHeight;
static int g_cropH, g_cropV;

int CreateWindow(const char *title);


void setBackgroundColor(float red, float green, float blue, float alpha);
void ClearScreen();

GLuint eggCompileShader(GLenum type, const char *shaderSrc);

GLuint eggShaderCreateProgram(GLuint vertexShaderObj, GLuint fragmentShaderObj);

GLint eggGetUniforms(GLuint program);

EGG_API void eggLogMessage(const char *formatStr, ...);

EGG_API char *eggLoadPCM(void *ioContext, const char *fileName, int *width, int *height);

void eggUnload();

GLuint GetGlowImage();

void UpdateWindow();
void EGG_Quit();

#ifdef __cplusplus
}
#endif

#endif