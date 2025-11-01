
#include "fractal_renderer.hpp"

#include <chrono>

#ifndef PaulBourke_Net

#include "pbcolor.hpp"

#endif

using namespace std;
using namespace std::chrono;

// Change params only in this block
namespace {
    const double width = 0.7;
    const double height = 3.5;

    double x = 0.0;
    double y = 0.0;
//  Works on 1440x900 resolution.
    const double aspect = 1440.0 / 900.0;

    const double minX = 0.5;
    const double minY = (minX + 0.5) * aspect;

    const double maxX = 5.5;
    const double maxY = (maxX + 0.5) * aspect;

    const double caw = maxX - minX;
    const double cah = maxY - minY;
    const double daw = width * 2 / caw;
    const double dah = height / cah;

    double t = 3.0;

    const double PHI = (1 + sqrt(5)) / 2;

    unsigned int totalFrames = 0;
    unsigned long totalTimeMS;
};

extern bool paused;


namespace /* std:: */ {
#define clock_now std::chrono::high_resolution_clock::now
};


namespace GLContext {
//   Intel i3 2.10Ghz with OpenGL 4.4 >
#define NUM_PARTICLES 20666

    static GLfloat vertexData[NUM_PARTICLES * 2];
    static GLfloat colorData[NUM_PARTICLES * 3];

// OpenGL ES 2.0 uses shaders

    GLint samplerLoc;
    GLint sensitivityLoc;

}; using namespace GLContext;


static void enableTexturing() {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetGlowImage());

    glUniform1i(samplerLoc, 0);
    glUniform1f(sensitivityLoc, 99.0f / 255.0f);

    glPointSize(32);
}

void RendererInit() {
    // Creates new OpenGL shader, (330 core)

//  Read vertex shader source.
    eggFile *fp = eggFileOpen(nullptr, "./basic.vs").filePointer;
    int vertexBytesLen = sizeof(char) * 1024;
    char *vertexShaderSrc = (char *) malloc(vertexBytesLen);

    int bytesRead = eggFileRead(fp, vertexBytesLen, vertexShaderSrc);

//  Compile the vertex shader.
    GLuint vertexShaderObj = eggCompileShader(GL_VERTEX_SHADER, vertexShaderSrc);

    eggFileClose(fp);
    free(vertexShaderSrc);

    if (vertexShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the vertex-shader [obj].\n");
        GL_CHECK();
        SDL_Quit();
    }

//  Read fragment source.
    fp = eggFileOpen(nullptr, "./basic.fs").filePointer;
    int fragmentBytesLen = sizeof(char) * 4096;
    char *fragmentShaderSrc = (char *) malloc(fragmentBytesLen);

    bytesRead = eggFileRead(fp, fragmentBytesLen, fragmentShaderSrc);
//  Compile the fragment shader.
    GLuint fragmentShaderObj = eggCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    eggFileClose(fp);
    free(fragmentShaderSrc);

    if (fragmentShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the fragment-shader [obj].\n");
        GL_CHECK();
        SDL_Quit();
    }

    GLuint program = eggShaderCreateProgram(vertexShaderObj, fragmentShaderObj);

//  Finally delete the vertex and fragment shader.
    glDeleteShader(vertexShaderObj);
    glDeleteShader(fragmentShaderObj);
    if (program == 0) {
        fprintf(stderr, "There was an error creating shader program.\n");
        GL_CHECK();
        SDL_Quit();
    }
    // Use the shader-program for the first time;
    glUseProgram(program);

    eggLogMessage("Uniforms: %d\n", eggGetUniforms(program));

    glDeleteProgram(program);

    samplerLoc = glGetUniformLocation(program, "s_texture");
    sensitivityLoc = glGetUniformLocation(program, "u_sensitivity");

    // Points position attribute to vertexData
    GLint position = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertexData);
    // Points color attribute to colorData
    GLint color = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 0, colorData);

    enableTexturing();
}

void updateTiming(const time_point<system_clock, milliseconds> lastFrameTime) {

    static unsigned int latency = 0;
    static unsigned long elapsedMS = 0;
    static unsigned int frames;

    const auto elapsed = std::chrono::duration_cast<milliseconds>(clock_now() - lastFrameTime).count();
    if (elapsed > latency) {
        latency = elapsed;
    }
    elapsedMS += elapsed;
    totalTimeMS += elapsed;

    if (elapsedMS >= 1000) {
        eggLogMessage("%d FPS, %dms lagged\n", frames, latency);
        frames = 0;
        elapsedMS = 0;
        latency = 0;
    }
    frames++;
    totalFrames++;
}

void Render() {
    const auto lastTime = clock_now();

    if (paused) return;
    ClearScreen();

//    Paul Dunn's Bubble Universe 3
//  Using REL's GlowImage <u>https://rel.phatcode.net</u>
    double j = 0;
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // PaulDunn, creator of SpecBasic, interpreter for SinClair Basic.
        const double u = sin(i + y) + sin(j / (NUM_PARTICLES * M_PI) + x);
        const double v = cos(i + y) + cos(j / (NUM_PARTICLES * M_PI) + x);
        x = u + t;
        y = v + t;

        const Color color = Color::createHue(
                cos(cos(i) - sin(t *PHI *PHI *PHI)));

        const auto vX = static_cast<GLfloat>((u - minX + 0.5) * daw);
        const auto vY = static_cast<GLfloat>((v - minY + 0.5) * dah + 0.5);

        const int vI = i * 2;
        vertexData[vI + 0] = vX;
        vertexData[vI + 1] = vY;

        const int cI = i * 3;
        colorData[cI + 0] = static_cast<GLfloat>(color.r);
        colorData[cI + 1] = static_cast<GLfloat>(color.g);
        colorData[cI + 2] = static_cast<GLfloat>(color.b);
        j += t;
    }
    t += 1.0 / 600.0;

    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

    updateTiming(std::chrono::time_point_cast<milliseconds, system_clock>( lastTime));

    UpdateWindow();

}

void Shutdown() {
    eggLogMessage("Rendered %d frames over %.2fs\n", totalFrames, (double)totalTimeMS / 1000.0);
}
