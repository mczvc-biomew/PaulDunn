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
    const int n = 200000;
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


namespace /* STD */ {
    #define clock_now std::chrono::high_resolution_clock::now
};


namespace GLContext {
static GLfloat vertexes[n << 1];
static GLfloat colors[n << 2];

// OpenGL ES 2.0 uses shaders
const char *VERTEX_SHADER = "#version 330 core\n"
	"attribute vec4 a_position;\n"
	"attribute vec4 a_color;\n"
	"varying vec4 v_color;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(a_position.xyz, 1.0);\n"
	"gl_PointSize = 1.0;\n"
	"v_color = a_color;\n"
	"}";

const char *FRAGMENT_SHADER = "#version 330 core\n"
	"precision mediump float;\n"
	"varying vec4 v_color;\n"
	"void main()\n"
	"{\n"
	"gl_FragColor = v_color;\n"
	"}";

}; using namespace GLContext;


void renderer_init()
{
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
	// Point color attribute to colors
	GLint color = glGetAttribLocation(program, "a_color");
	glEnableVertexAttribArray(color);
	glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, 0, colors);

}

void render()
{
    if (paused) return;
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//    clearScreen();

	for (int i = 0, j = 0; i < n; i++, j+=t)
	{
	    // PaulDunn, creator of SpecBasic
		const double u = sin(i+y) + sin(j / (n * M_PI) + x);
		const double v = cos(i+y) + cos(j / (n * M_PI) + x);
		x = u + t;
		y = v + t;

		const Color color = Color::createHue(cos(cos(i) - sin(t)));

		const int jj = i*2;
        const double v1 = static_cast<GLfloat>((u - minX + 0.5) * daw - 0.25);
        const double v2 = static_cast<GLfloat>((v - minY + 0.5) * dah - 0.25);

		vertexes[jj + 0] = v1;
		vertexes[jj + 1] = v2;

		colors[i*4 + 0] = static_cast<GLfloat>(color.r);
		colors[i*4 + 1] = static_cast<GLfloat>(color.g);
		colors[i*4 + 2] = static_cast<GLfloat>(color.b);
		colors[i*4 + 3] = 99.0/255.0;

    }
	t += 1.0/60.0;

	glDrawArrays(GL_POINTS, 0, n);
    updateWindow();

}
