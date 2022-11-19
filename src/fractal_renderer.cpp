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

    double x = 0.0;
    double y = 0.0;

    const double aspect = 1520.0 / 720.0;

    const double minX = 0.0001;
    const double minY = (minX /*+ 0.5*/) * aspect;

    const double maxX = 3.5;
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
#define NUM_PARTICLES 9560

static GLfloat vertexData[NUM_PARTICLES * 2];
static GLfloat colorData[NUM_PARTICLES * 3];

// OpenGL ES 2.0 uses shaders
const char *VERTEX_SHADER =
	"attribute vec4 a_position;\n"
	"attribute vec4 a_color;\n"
	"varying vec4 v_color;\n"
	"void main()\n"
	"{\n"
#if defined(__ANDROID__)
    "gl_PointSize = 64.0;\n"
#endif
	"   gl_Position = vec4(a_position.xyz, 1.0);\n"
	"   v_color = a_color;\n"
	"}";

const char *FRAGMENT_SHADER =
	"precision mediump float;\n"
	"varying vec4 v_color;\n"
#if defined(__ANDROID__)
    "layout(location = 0) out vec4 fragColor;\n"
#endif
    "uniform float u_sensitivity;\n"
    "uniform sampler2D s_texture;\n"
	"void main()\n"
	"{\n"
    "   vec4 texColor = texture(s_texture, gl_PointCoord);\n"
#if defined(__ANDROID__)
    "   gl_FragColor = vec4(v_color.rgb, u_sensitivity) * texColor.bgra;\n"
#else
    "   fragColor = vec4(v_color.rgb, u_sensitivity) * texColor.bgra;\n"
#endif
	"}";

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

#if !defined(__ANDROID__)
    glPointSize(64);
#endif
}

void RendererInit()
{
    GLuint program = loadShaderProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (program == 0) {
        SDL_Quit();
    }
    // Use it
    glUseProgram(program);

    samplerLoc = glGetUniformLocation(program, "s_texture");
    sensitivityLoc = glGetUniformLocation(program, "u_sensitivity");

    // Point position attribute to vertexData
    GLint position = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, vertexData);
    // Point color attribute to colorData
    GLint color = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 0, colorData);

    enableTexturing();
}

void Render()
{

    if (paused) return;
    ClearScreen();

//   Paul Dunn's Bubble Universe 3
//    Using REL's GlowImage <u>https://rel.phatcode.net</u>
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

        colorData[i * 3 + 0] = static_cast<GLfloat>(color.r);
        colorData[i * 3 + 1] = static_cast<GLfloat>(color.g);
        colorData[i * 3 + 2] = static_cast<GLfloat>(color.b);
    }
    t += 1.0/60.0;

	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    UpdateWindow();

}
