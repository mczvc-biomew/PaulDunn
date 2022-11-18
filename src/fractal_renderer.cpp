#include "fractal_renderer.hpp"
#include <chrono>

#include "fractal_renderer.hpp"

#ifndef PaulBourke_Net
#include "pbcolor.hpp"
#endif

using namespace std;
using namespace std::chrono;

// Change params only in this block
namespace
{
    const double width = 0.8;
    const double height = 4.0;
    int frames = 1;

    double x = 0.0;
    double y = 0.0;

    const double aspect = 1920.0 / 1100.0;

    const double minX = 0.0001;
    const double minY = (minX /*+ 0.5*/) * aspect;

    const double maxX = 4.0;
    const double maxY = (maxX + 1.0) * aspect;

    const double caw = maxX - minX;
    const double cah = maxY - minY;
    const double daw = width * 2 / caw;
    const double dah = height / cah;
  
    double t = 9.0;
      
};

extern bool paused;


namespace /* std:: */ {
    #define clock_now std::chrono::high_resolution_clock::now
};


namespace GLContext {
#define NUM_PARTICLES 25600

static GLfloat vertexData[NUM_PARTICLES * 2];
static GLfloat colorData[NUM_PARTICLES * 4];

// OpenGL ES 2.0 uses shaders
const char *VERTEX_SHADER = "#version 330 core\n"
	"attribute vec4 a_position;\n"
	"attribute vec4 a_color;\n"
	"varying vec4 v_color;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(a_position.xyz, 1.0);\n"
	"   v_color = a_color;\n"
	"}";

const char *FRAGMENT_SHADER = "#version 330 core\n"
	"precision mediump float;\n"
	"varying vec4 v_color;\n"
    "layout(location = 0) out vec4 fragColor;\n"
    "uniform sampler2D s_texture;\n"
	"void main()\n"
	"{\n"
    "   vec4 texColor = texture(s_texture, gl_PointCoord);\n"
    "   fragColor = vec4(v_color) * texColor;\n"
	"}";

GLint samplerLoc;

}; using namespace GLContext;


static GLuint loadShader(GLenum type, const char* shaderSrc) {
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

static GLuint loadProgram(const char *vertexShaderSrc, const char *fragShaderSrc) {
    GLint linked;
    // Load shaders
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexShaderSrc);

    if (vertex == 0) {
        fprintf(stderr, "There's an error compiling the vertex shader.\n");
        return 0;
    }

    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragShaderSrc);

    if (fragment == 0) {
        fprintf(stderr, "There's an error compiling the fragment shader.\n");
        return 0;
    }

    // Combine shaders into program
    GLuint program = glCreateProgram();

    if (program == 0) {
        fprintf(stderr, "There's an error creating shader program.\n");
        return 0;
    }

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    // Link the program
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > -1) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            fprintf(stderr, "Error linking program\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}


static void enableTexturing() {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetGlowImage());

    glUniform1i(samplerLoc, 0);

    glPointSize(80);
}

void rendererInit()
{
    GLuint program = loadProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (program == 0) {
        SDL_Quit();
    }
    // Use it
    glUseProgram(program);

    samplerLoc = glGetUniformLocation(program, "s_texture");

    // Point position attribute to vertexData
    GLint position = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertexData);
    // Point color attribute to colorData
    GLint color = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, 0, colorData);

    enableTexturing();
}

void Render()
{

    if (paused) return;
    clearScreen();

    for (int i = 0, j = 0; i < NUM_PARTICLES; i++, j+=t)
    {
        // PaulDunn, creator of SpecBasic
        const double u = sin(i+y) + sin(j / (NUM_PARTICLES * M_PI) + x);
        const double v = cos(i+y) + cos(j / (NUM_PARTICLES * M_PI) + x);
        x = u + t;
        y = v + t;

        const Color color = Color::createHue(cos(cos(i) - sin(t)));

        const int jj = i*2;
        const double v1 = static_cast<GLfloat>((u - minX + 0.5) * daw - 0.25);
        const double v2 = static_cast<GLfloat>((v - minY + 0.5) * dah - 0.25);

        vertexData[jj + 0] = v1;
        vertexData[jj + 1] = v2;

        colorData[i * 4 + 0] = static_cast<GLfloat>(color.r);
        colorData[i * 4 + 1] = static_cast<GLfloat>(color.g);
        colorData[i * 4 + 2] = static_cast<GLfloat>(color.b);
        colorData[i * 4 + 3] = 99.0 / 255.0;

    }
    t += 1.0/60.0;
    
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    updateWindow();

}
