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

const char *VERTEX_SHADER_ES3 = "#version 330 core\n"
    "uniform float u_time;\n"
    "uniform vec3 u_centerPosition;\n"
    "layout(location = 0) in float a_lifetime;\n"
    "layout(location = 1) in vec3 a_startPosition;\n"
    "layout(location = 2) in vec3 a_endPosition;\n"
    "out float v_lifetime;\n"
    "void main()\n"
    "{\n"
    "   if (u_time <= a_lifetime)\n"
    "   {\n"
    "       gl_Position.xyz = a_startPosition + (u_time * a_endPosition);\n"
    "       gl_Position.xyz += u_centerPosition;\n"
    "       gl_Position.w = 1.0;\n"
    "   } else\n"
    "   {\n"
    "       gl_Position = vec4(-1000, -1000, 0, 0);\n"
    "   }\n"
    "   v_lifetime = 1.0 - (u_time / a_lifetime);\n"
    "   v_lifetime = clamp(v_lifetime, 0.0, 1.0);\n"
    "   gl_PointSize = (v_lifetime * v_lifetime) * 40.0;\n"
    "}\n";

const char *FRAGMENT_SHADER_ES3 = "#version 330 core\n"
    "precision mediump float;\n"
    "uniform vec4 u_color;\n"
    "in float v_lifetime;\n"
    "layout(location = 0) out vec4 fragColor;\n"
    "uniform sampler2D s_texture;\n"
    "void main()\n"
    "{\n"
    "   vec4 texColor;\n"
    "   texColor = texture(s_texture, gl_PointCoord);\n"
    "   fragColor = vec4(u_color) * texColor;\n"
    "   fragColor.a *= v_lifetime;\n"
    "}\n";

GLint timeLoc;
GLint colorLoc;
GLint centerPositionLoc;
GLint samplerLoc;

#define NUM_PARTICLES 20000
#define PARTICLE_SIZE 7

#define ATTRIBUTE_LIFETIME_LOCATION 0
#define ATTRIBUTE_STARTPOSITION_LOCATION 1
#define ATTRIBUTE_ENDPOSITION_LOCATION 2

float particleData[NUM_PARTICLES * PARTICLE_SIZE];

float time;

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
//        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint loadProgram(const char *vertexShaderSrc, const char *fragShaderSrc) {
    GLint linked;
    // Load shaders
    GLuint vertex = loadShader(GL_VERTEX_SHADER, vertexShaderSrc);
//            glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertex, 1, &VERTEX_SHADER_ES3, NULL);
//    glCompileShader(vertex);

    if (vertex == 0) {
        fprintf(stderr, "There's an error compiling the vertex shader.\n");
        return 0;
    }

    GLuint fragment = loadShader(GL_FRAGMENT_SHADER, fragShaderSrc);
//            glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragment, 1, &FRAGMENT_SHADER_ES3, NULL);
//    glCompileShader(fragment);
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

//    glDeleteShader(vertex);
//    glDeleteShader(fragment);

    // Use it
    glUseProgram(program);

    return program;
}

static void loadParticles() {
    srand(0);
    float centerPos[3];
    float color[4];

    centerPos[0] = 0.0f;
    centerPos[1] = 0.0f;
    centerPos[2] = ((float)(rand() % 10000) / 10000.0f) - 0.5f;

    glUniform3fv(centerPositionLoc, 1, &centerPos[0]);

    // Random color
    color[0] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
    color[1] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
    color[2] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
    color[3] = 0.5f;

    glUniform4fv(colorLoc, 1, &color[0]);

    GLContext::time = 1.0f;
//    t += 1.0/60.0;
}

void renderInit3() {
    GLuint program = loadProgram(VERTEX_SHADER_ES3, FRAGMENT_SHADER_ES3);

    timeLoc = glGetUniformLocation(program, "u_time");
    centerPositionLoc = glGetUniformLocation(program, "u_centerPosition");
    colorLoc = glGetUniformLocation(program, "u_color");
    samplerLoc = glGetUniformLocation(program, "s_texture");

    GLint PARTICLE_DEPTH = PARTICLE_SIZE * sizeof(GLfloat);

    // Point Lifetime attribute to particleData[0]
    glEnableVertexAttribArray(ATTRIBUTE_LIFETIME_LOCATION);
    glVertexAttribPointer(ATTRIBUTE_LIFETIME_LOCATION, 1, GL_FLOAT, GL_FALSE,
                          PARTICLE_DEPTH, particleData);
    // Point EndPosition attribute to particleData[1]
    glEnableVertexAttribArray(ATTRIBUTE_ENDPOSITION_LOCATION);
    glVertexAttribPointer(ATTRIBUTE_ENDPOSITION_LOCATION, 3, GL_FLOAT, GL_FALSE,
                          PARTICLE_DEPTH, &particleData[1]);
    // Point StartPosition attribute to particleData[4]
    glEnableVertexAttribArray(ATTRIBUTE_STARTPOSITION_LOCATION);
    glVertexAttribPointer(ATTRIBUTE_STARTPOSITION_LOCATION, 3, GL_FLOAT, GL_FALSE,
                          PARTICLE_DEPTH, &particleData[4]);

    loadParticles();
}

void Update(float deltaTime)
{
    if (paused) return;
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//    clearScreen();

//	for (int i = 0, j = 0; i < n; i++, j+=t)
//	{
//	    // PaulDunn, creator of SpecBasic
//		const double u = sin(i+y) + sin(j / (n * M_PI) + x);
//		const double v = cos(i+y) + cos(j / (n * M_PI) + x);
//		x = u + t;
//		y = v + t;
//
//		const Color color = Color::createHue(cos(cos(i) - sin(t)));
//
//		const int jj = i*2;
//        const double v1 = static_cast<GLfloat>((u - minX + 0.5) * daw - 0.25);
//        const double v2 = static_cast<GLfloat>((v - minY + 0.5) * dah - 0.25);
//
//		vertexes[jj + 0] = v1;
//		vertexes[jj + 1] = v2;
//
//		colors[i*4 + 0] = static_cast<GLfloat>(color.r);
//		colors[i*4 + 1] = static_cast<GLfloat>(color.g);
//		colors[i*4 + 2] = static_cast<GLfloat>(color.b);
//		colors[i*4 + 3] = 99.0/255.0;
//
//    }
//	t += 1.0/60.0;

    GLContext::time += deltaTime;

    if (GLContext::time >= 1.0f) {


        GLContext::time = 0.0f;

        // PaulDunn, creator of SpecBasic
        for (int i = 0, j = 0; i < NUM_PARTICLES; i++, j+=t)
        {
            float *particleDataP = &particleData[i * PARTICLE_SIZE];

            // PaulDunn, creator of SpecBasic
            const double u = sin(i+y) + sin(j / (NUM_PARTICLES * M_PI) + x);
            const double v = cos(i+y) + cos(j / (NUM_PARTICLES * M_PI) + x);
            x = u + t;
            y = v + t;

            //const Color color = Color::createHue(cos(cos(i) - sin(t)));

            // Lifetime of a particle
            (*particleDataP++) = ((float)(rand() % 10000) / 10000.0f);

            const auto v1 = static_cast<float>((u - minX + 0.5f) * daw - 0.25f);
            const auto v2 = static_cast<float>((v - minY + 0.5f) * dah - 0.25f);

            const double w = sin(i*NUM_PARTICLES+y) + sin(j*NUM_PARTICLES / (NUM_PARTICLES * M_PI) + x);
            const double z = cos(i*NUM_PARTICLES+y) + cos(j*NUM_PARTICLES / (NUM_PARTICLES * M_PI) + x);

            const auto v3 = static_cast<float>((w - minX + 0.5f) * daw - 0.25f);
            const auto v4 = static_cast<float>((z - minY + 0.5f) * dah - 0.25f);
            // End position of a particle
            (*particleDataP++) = v3;
            (*particleDataP++) = v4;
            (*particleDataP++) = ( (float)(rand() % 10000)/ 5000.0f) - 1.0f;

            // Start position of a particle
            (*particleDataP++) = v1;
            (*particleDataP++) = v2;
//        (*particleDataP++) = ( (float)(rand() % 10000) / 40000.0f) - 0.125f;
//        (*particleDataP++) = ( (float)(rand() % 10000) / 40000.0f) - 0.125f;
            (*particleDataP++) = ( (float)(rand() % 10000) / 40000.0f) - 0.125f;

            /*colors[i*4 + 0] = static_cast<GLfloat>(color.r);
            colors[i*4 + 1] = static_cast<GLfloat>(color.g);
            colors[i*4 + 2] = static_cast<GLfloat>(color.b);
            colors[i*4 + 3] = 99.0/255.0;*/

        }

    }
    t += 1.0/60.0;

    glUniform1f(timeLoc, GLContext::time);
}

void Render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetGlowImage());

    glUniform1i(samplerLoc, 0);
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    updateWindow();

}
