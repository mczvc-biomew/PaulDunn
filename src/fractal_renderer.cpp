
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
    const char *VERTEX_SHADER = "#version 330 core\n"
                                "attribute vec4 a_position;\n"
                                "attribute vec4 a_color;\n"
                                "varying vec4 v_color;\n"
                                "void main()\n"
                                "{\n"
                                "   gl_Position = vec4(a_position.xyz, 1.0);\n"
                                "   gl_PointSize = 64.0;\n"
                                "   v_color = a_color;\n"
                                "}";

    const char *FRAGMENT_SHADER = "#version 330 core\n"
                                  "precision mediump float;\n"
                                  "varying vec4 v_color;\n"
                                  "layout(location = 0) out vec4 fragColor;\n"
                                  "uniform float u_sensitivity;\n"
                                  "uniform sampler2D s_texture;\n"
                                  "void main()\n"
                                  "{\n"
                                  "    vec4 texColor = texture(s_texture, gl_PointCoord);\n"
                                  "    \n"
                                  "    float alpha = (v_color.r + v_color.g + v_color.b) / 3.0;\n"
                                  "\n"
                                  "    vec4 c = vec4(v_color.rgb, 1.0 - alpha) * vec4(texColor.rgb, texColor.a);\n"
                                  "    vec4 d = c;\n"
                                  "\n"
                                  "    if (true) {\n"
                                  "        float threshold = 0.003;\n"
                                  "\n"
                                  "        if ( ((c.r + c.g + c.b + c.a)/4.0) > threshold ) {\n"
                                  "            d = vec4( c.rgb * 5.5, u_sensitivity * c.a );\n"
                                  "         // c = vec4( c.rgb * 0.0001, 1.0 - c.a * 0.001 * u_sensitivity);\n"
                                  "\n"
                                  "            float luminance = dot(c.rgb, vec3(1.2126, 1.7152, 1.0722));\n"
                                  "            vec3 toneMappedColor = c.rgb / (c.rgb + vec3(1.0));\n"
                                  "            toneMappedColor *= luminance / dot(toneMappedColor, vec3(0.2126, 1.7152, 0.0722));\n"
                                  "\n"
                                  "            c = vec4(c.rgb * toneMappedColor, c.a);\n"
                                  "        }\n"
                                  "    }\n"
                                  "\n"
                                  "    fragColor = vec4( c.rgb * d.rgb, 1.0 - d.a);\n"
                                  "    //fragColor = vec4(v_color.rgb, u_sensitivity) * texColor.bgra;\n"
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

    glPointSize(32);
}

void RendererInit() {
    // Creates new OpenGL shader, (330 core)
    GLuint program = eggLoadShaderProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (program == 0) {
        SDL_Quit();
    }
    // Use it
    glUseProgram(program);
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
