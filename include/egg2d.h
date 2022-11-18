#ifndef EGG_H_INCLUDED
/**
 * @file egg2d.c
 * Easy Game Graphics 0.1
 *
 * [mczvc] (2022) <czarm827@protonmail.com>
 * Meldencio Czarlemagne Veras Corrales, CS (2nd year)
 *
 * https://github.com/mczvc827
 */
#define EGG_H_INCLUDED

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>


extern int g_targetWidth, g_targetHeight;
extern double g_targetAspect;
static int g_actualWidth, g_actualHeight;
static int g_aspect;
static int g_scale;
static int g_scaledWidth, g_scaledHeight;
static int g_cropH, g_cropV;

int createWindow(const char* title);

void clearScreen();

GLuint loadShader(GLenum type, const char *shaderSrc);

GLuint loadShaderProgram(const char *vertexShaderSrc, const char *fragShaderSrc);

GLuint GetGlowImage();

void updateWindow();
void EGG_Quit();

#endif