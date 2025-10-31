
#include "chaosgame.hpp"

using namespace std;
using namespace std::chrono;

class AttractorSet {
private:
    double x, y;
public:
    AttractorSet(double x, double y): x(x), y(y) {}

    void setX(double value) { x = value; }
    void setY(double value) { y = value; }

    double getX() { return x; }
    double getY() { return y; }

};

class AlphaAttractor: public AttractorSet {
private:
    double a, b, c, d;
    int iters;
public:
    AlphaAttractor(double x, double y,
                   double a, double b, double c, double d, int iters):
            AttractorSet(x, y), a(a), b(b), c(c), d(d), iters(iters) { }
    double getA() const { return a; }
    double getB() const { return b; }
    double getC() const { return c; }
    double getD() const { return d; }

    int getIterations() const { return iters; }

    void setA(double value) { a = value; }
    void setB(double value) { b = value; }
    void setC(double value) { c = value; }
    void setD(double value) { d = value; }

    void updateParams(double a, double b, double c, double d) {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }

};

namespace CGameGLContext {
// OpenGL ES 2.0 uses shaders
//   Intel i3 2.10Ghz with OpenGL 4.4 >
    constexpr int NUM_PARTICLES = 906600;

    static GLfloat vertexData[NUM_PARTICLES * 2];
    static GLfloat prevVertexData[NUM_PARTICLES * 2];
    static GLint idData[NUM_PARTICLES];


    GLint samplerLoc;
    GLint sensitivityLoc;
    GLint angleLoc;

};

namespace {

    AlphaAttractor dream {0.1, 0.1, -0.976918, 2.870979, 0.718145, 0.642928,
                          CGameGLContext::NUM_PARTICLES};

    double x = dream.getX();
    double y = dream.getY();
    const double PHI = (1 + sqrt(5)) / 2;

    const double width = 0.7;
    const double height = 1.35;
    const double aspect = 600.0 / 600.0;

    const double minX = 0.5;
    const double minY = (minX + 0.85) * aspect;

    const double maxX = 3.0;
    const double maxY = (maxX + 0.5) * aspect;

    const double caw = maxX - minX;
    const double cah = maxY - minY;
    const double daw = width * 2 / caw;
    const double dah = height / cah;
    double t = 3.0;

    const double aBounds = 5.1;

    unsigned int totalFrames = 0;
    unsigned int frameCounter = 0;
    unsigned long totalTimeMS;
}

extern bool paused;

namespace /* std:: */ {
#define clock_now std::chrono::high_resolution_clock::now
};

static void step() {

//    Clifford Pickover's Attractor
//  Using REL's GlowImage <u>https://rel.phatcode.net</u>
    double j = 0;
    CGameGLContext::prevVertexData[0] = (float)x;
    CGameGLContext::prevVertexData[1] = (float)y;
    static double a = dream.getA();
    for (int i = 0; i < dream.getIterations(); i++) {
        // PaulDunn, creator of SpecBasic, interpreter for SinClair Basic.
        const double u = std::sin(y * dream.getB()) + dream.getC()*std::sin(x * dream.getB());
        const double v = std::sin(x * a) + dream.getD()*std::sin(y * a);

        x = u;
        y = v;

        const auto vX = static_cast<GLfloat>((x - minX + 0.5) * daw);
        const auto vY = static_cast<GLfloat>((y - minY + 0.5) * dah + 0.5);

        const int pvI = (i - 1) * 2;
        if (i > 1) {
            CGameGLContext::prevVertexData[pvI + 0] = vX;
            CGameGLContext::prevVertexData[pvI + 1] = vY;
        }

        const int vI = i * 2;

        CGameGLContext::vertexData[vI + 0] = vX;
        CGameGLContext::vertexData[vI + 1] = vY;

        CGameGLContext::idData[vI] = i;

        j += t;

    }
    static double tDir = 1.0 / 600.0;
    a = a + tDir *24.0*10.5* sin(sin(t) * M_PI/32.0);
//    printf("%f\n", dream.getA());
//    dream.setB(dream.getB() + 0.1 * sin(sin(t) * M_PI/32.0));
    glUniform1f(CGameGLContext::angleLoc, (float)std::sin(t *PHI *PHI *PHI));
    dream.setX(x);
    dream.setY(y);
    if (a - aBounds < -(aBounds + 5.0)) {
//        printf("%f\n", a - aBounds);
        tDir *= -1.0;
    }
    t += tDir;
}

static void enableTexturing() {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GetGlowImage());

    glUniform1i(CGameGLContext::samplerLoc, 0);
    glUniform1f(CGameGLContext::sensitivityLoc, 10.0f / 255.0f);

    glPointSize(13);
}

void InitCGame() {

    // Creates new OpenGL shader, (330 core)

//  Read vertex shader source.
    eggFile *fp = eggFileOpen(nullptr, "./chaos.vs");
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
    fp = eggFileOpen(nullptr, "./chaos.fs");
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

    CGameGLContext::samplerLoc = glGetUniformLocation(program, "s_texture");
    CGameGLContext::sensitivityLoc = glGetUniformLocation(program, "u_sensitivity");
    CGameGLContext::angleLoc = glGetUniformLocation(program,"u_angle");

    // Points position attribute to vertexData
    GLint position = glGetAttribLocation(program, "a_pos");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, CGameGLContext::vertexData);
    GLint prevPos = glGetAttribLocation(program, "a_prevpos");
    glEnableVertexAttribArray(prevPos);
    glVertexAttribPointer(prevPos, 2, GL_FLOAT, GL_FALSE, 0, &CGameGLContext::prevVertexData);
    GLint id = glGetAttribLocation(program, "a_id");
    glEnableVertexAttribArray(id);
    glVertexAttribPointer(id, 1, GL_INT, GL_TRUE, 0, CGameGLContext::idData);

    enableTexturing();
    step();
}


static void updateTiming(const time_point<system_clock, milliseconds> lastFrameTime) {

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

/**
 * Render loop.
 */
void RenderCGame() {
    static chrono::system_clock::time_point lastTime = clock_now();
//    const long timeDuration = chrono::duration_cast<milliseconds>(
//            clock_now() - chrono::time_point_cast<milliseconds , system_clock>(
//                    lastTime)).count();
//    const double timeSecs = (double)timeDuration * 0.001;

    if (paused) return;

    ClearScreen();
    glDrawArrays(GL_POINTS, 0, CGameGLContext::NUM_PARTICLES);
    UpdateWindow();
//        printf("%f %d\n", timeSecs, totalFrames);
    lastTime = clock_now();
    step();
    frameCounter += 1;

    updateTiming(std::chrono::time_point_cast<milliseconds, system_clock>( lastTime));

//    paused = true;

}

void ShutdownCGame() {
    eggLogMessage("Rendered %d frames over %.2fs\n", totalFrames, (double)totalTimeMS / 1000.0);
}