
#include "chaosgame.hpp"

using namespace std;
using namespace std::chrono;

//#define TEST_ONE

class AttractorSet {
private:
    double x, y;
public:
    AttractorSet(double x, double y): x(x), y(y) {}

    void setX(double value) { x = value; }
    void setY(double value) { y = value; }

    double getX() const { return x; }
    double getY() const { return y; }

};

class AlphaAttractor: public AttractorSet {
private:
    double m_a, m_b, m_c, m_d;
    int iters;
public:
    AlphaAttractor(double x, double y,
                   double a, double b, double c, double d, int iters):
            AttractorSet(x, y), m_a(a), m_b(b), m_c(c), m_d(d), iters(iters) { }
    double getA() const { return m_a; }
    double getB() const { return m_b; }
    double getC() const { return m_c; }
    double getD() const { return m_d; }

    int getIterations() const { return iters; }

    void setA(double value) { m_a = value; }
    void setB(double value) { m_b = value; }
    void setC(double value) { m_c = value; }
    void setD(double value) { m_d = value; }

    void updateParams(double a, double b, double c, double d) {
        this->m_a = a;
        this->m_b = b;
        this->m_c = c;
        this->m_d = d;
    }

};

namespace CGameGLContext {
// OpenGL ES 2.0 uses shaders
//   Intel i3 2.10Ghz with OpenGL 4.4 >
    constexpr int NUM_PARTICLES = 110240
#ifdef TEST_ONE
    * 1;
#else
    * 3;
#endif

    // Packed data storage is better than non-contiguous memory layout!
    static GLfloat attractor2Data[NUM_PARTICLES * (2 + 2)];
    static GLint idData[NUM_PARTICLES];

    GLint samplerLoc;
    GLint sensitivityLoc;
    GLint angleLoc;

};

namespace Parameters {

    AlphaAttractor dream {0.1, 0.1,
                          -0.976918, 2.870979,
                          0.718145, 0.642928,
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

    const double aUpperBounds = 5.1;
    const double aLowerBounds = 24.5;
    constexpr uint screenBackPressure = 10000000;
    bool isScreenDirty = false;

    uint dataSent = 0;

    unsigned int totalFrames = 0;
    unsigned int frameCounter = 0;
    unsigned long totalTimeMS;
}
using namespace Parameters;

extern bool paused;

namespace /* std:: */ {
#define clock_now std::chrono::high_resolution_clock::now
};

/**  Clifford Pickover's Attractor
 *  ------------------------------- \n
 *  Using REL's GlowImage <u>https://rel.phatcode.net</u>
 */
static void step() {

    CGameGLContext::attractor2Data[0] = (float)x;
    CGameGLContext::attractor2Data[1] = (float)y;
    CGameGLContext::idData[0] = 0;
    static double a = dream.getA();
    for (int i = 1; i < dream.getIterations(); i++) {
        const double u = std::sin(y * dream.getB()) + dream.getC()*std::sin(x * dream.getB());
        const double v = std::sin(x * a) + dream.getD()*std::sin(y * a);

        x = u;
        y = v;

        const auto vX = static_cast<GLfloat>((x - minX + 0.5) * daw);
        const auto vY = static_cast<GLfloat>((y - minY + 0.5) * dah + 0.5);

        const int vI = i * 4;
//        const int vI = (i + 1) * 4;
        CGameGLContext::attractor2Data[vI - 2] = vX;
        CGameGLContext::attractor2Data[vI - 1] = vY;

        if (i < dream.getIterations() - 1) {
            CGameGLContext::attractor2Data[vI + 0] = vX;
            CGameGLContext::attractor2Data[vI + 1] = vY;
        }

        CGameGLContext::idData[i] = i;

    }
    static double tDir = 1.0 / 600.0;
    const double aDelta = abs(a - aUpperBounds);
    constexpr double EPSILON = 0.01;

    if (dataSent > screenBackPressure * 0.005
#ifdef TEST_ONE
    *1
#else
    *3
#endif
    ) {
        a = a + tDir * 24.0 * 3.125 * sin(sin(t) * M_PI / 32.0);
        glUniform1f(CGameGLContext::angleLoc, (float)std::sin(t *PHI *PHI *PHI));
        dream.setX(x);
        dream.setY(y);
//        printf("%f\n", abs(aDelta - aLowerBounds));
    }
//    printf("%f\n", dream.getA());
//    dream.setB(dream.getB() + 0.1 * sin(sin(t) * M_PI/32.0));
    if (aDelta < EPSILON || abs(aDelta - aLowerBounds) > aLowerBounds) {
        tDir = -(tDir - 1.0);
    }
    t += tDir;
    isScreenDirty = false;
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
    struct EggFileContext eggFile = eggFileOpen(nullptr, "./chaos.vs");
    long vertexBytesLen = (long)sizeof(char) * eggFile.size;
    char *vertexShaderSrc = (char *) malloc(vertexBytesLen);

    size_t bytesRead = eggFileRead(eggFile.filePointer, vertexBytesLen, vertexShaderSrc);
    vertexShaderSrc[bytesRead] = '\0';
//    printf("%zu: %s\n", bytesRead, vertexShaderSrc);
//    printf("%d", vertexShaderSrc[bytesRead-1]);

//  Compile the vertex shader.
    GLuint vertexShaderObj = eggCompileShader(GL_VERTEX_SHADER, vertexShaderSrc);

    eggFileClose(eggFile.filePointer);
    free(vertexShaderSrc);

    if (vertexShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the vertex-shader [obj].\n");
#if defined(DEBUG)
        printf("%ld bytes\n", eggFile.size);
#endif
        GL_CHECK();
        SDL_Quit();
    }

//  Read fragment source.

    eggFile = eggFileOpen(nullptr, "./chaos.fs");
    long fragmentBytesLen = (long)sizeof(char) * eggFile.size;
    char *fragmentShaderSrc = (char *) malloc(fragmentBytesLen);

    bytesRead = eggFileRead(eggFile.filePointer, fragmentBytesLen, fragmentShaderSrc);
    fragmentShaderSrc[bytesRead] = '\0';
//    printf("%d", fragmentShaderSrc[bytesRead-1]);
//  Compile the fragment shader.
    GLuint fragmentShaderObj = eggCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    eggFileClose(eggFile.filePointer);
    free(fragmentShaderSrc);

    if (fragmentShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the fragment-shader [obj].\n");
#if defined(DEBUG)
        printf("%ld bytes\n", eggFile.size)
#endif
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

#ifdef DEBUG
    eggLogMessage("Uniforms: %d\n", eggGetUniforms(program));
#endif

    glDeleteProgram(program);

    CGameGLContext::samplerLoc = glGetUniformLocation(program, "s_texture");
    CGameGLContext::sensitivityLoc = glGetUniformLocation(program, "u_sensitivity");
    CGameGLContext::angleLoc = glGetUniformLocation(program,"u_angle");

    printf("Using %luMBs +\n", (sizeof(CGameGLContext::attractor2Data) / (1000*1000)
    + sizeof(CGameGLContext::idData) / (1000*1000)));

    // Attractor position and previous position attribute on shader
    GLint attractor = glGetAttribLocation(program, "a_data");
    glEnableVertexAttribArray(attractor);
    glVertexAttribPointer(attractor, 4, GL_FLOAT, GL_FALSE, 0,
                          CGameGLContext::attractor2Data);
    GLint id = glGetAttribLocation(program, "a_id");
    glEnableVertexAttribArray(id);
    glVertexAttribPointer(id, 1, GL_INT, GL_TRUE, 0, CGameGLContext::idData);

    enableTexturing();
    setBackgroundColor(0.0f, 0.2f, 0.2f, 0.0f);
    ClearScreen();
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
bool RenderCGame() {
    static chrono::system_clock::time_point lastTime = clock_now();
    static chrono::system_clock::time_point lastDrawTime = clock_now();


#if defined(TEST_ONE) || defined(TEST_THREE)
    const long timeDuration = chrono::duration_cast<milliseconds>(
            clock_now() - chrono::time_point_cast<milliseconds, system_clock>(
                    lastTime)).count();
    const double timeSecs = (double) timeDuration * 0.001;
#endif

    if (paused) return true;

    if (dataSent > screenBackPressure) {
        ClearScreen();
        dataSent -= screenBackPressure;
        isScreenDirty = true;
    }
    UpdateWindow();
    step(); // this uses 20% of CPU (margin of -2% !!)
    glDrawArrays(GL_POINTS, 0, CGameGLContext::NUM_PARTICLES);
    dataSent += dream.getIterations();
    frameCounter += 1;

    updateTiming(std::chrono::time_point_cast<milliseconds, system_clock>( lastDrawTime));
    lastDrawTime = clock_now();
#if defined(TEST_ONE) || defined(TEST_THREE)
//    printf("%lf\n", timeSecs);
    if (timeSecs > 10) {
        paused = true;
        return false;
    }
#endif
    return true;
}

void ShutdownCGame() {
    eggLogMessage("Rendered %d frames over %.2fs, average of %.2f FPS..\n",
                  totalFrames, (double)totalTimeMS / 1000.0,
                  totalFrames / ((double)totalTimeMS*0.001));
}