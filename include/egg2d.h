#ifndef EGG_H_INCLUDED
/**
 * @file egg2d.h\n
 * Easy Game Graphics v0.3.4
 *
 * (c) 2022-2025 [mczvc](mailto:mczvc\@proton.me)<br><br>
 * <b>Meldencio Czarlemagne Corrales</b>,
 *
 * <li> https://github.com/mczvc-biomew </li>
 * <li> https://mczvc-biomew.github.io/mczvc </li>
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


EGG_API eggFile *eggFileOpen(void *ioContext, const char *fileName);
EGG_API void eggFileClose(eggFile *pFile);
EGG_API size_t eggFileRead(eggFile *pFile, int bytesToRead, void *buffer);

extern int g_targetWidth, g_targetHeight;
extern double g_targetAspect;

static int g_actualWidth, g_actualHeight;
static int g_aspect;
static float g_scale;
static int g_scaledWidth, g_scaledHeight;
static int g_cropH, g_cropV;

int CreateWindow(const char *title);

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