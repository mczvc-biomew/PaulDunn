/**
 * > @file egg2d.c\n
 * Easy Game Graphics v0.3.4
 *
 * [Maintainer]:
 *
 * (c) 2022-2025 [mczvc](mailto:mczvc\@proton.me)<br><br>
 * <b>Meldencio Czarlemagne Corrales</b>,
 *
 * <li> https://github.com/mczvc-biomew </li>
 * <li> https://mczvc-biomew.github.io/mczvc </li><br>
 *
 * <h2>[PicoSoft]:</h2>
 *
 * <li> https://mczvc-biomew.github.io/picosoft </li>
 */
#define EGG_VERSION 0x000305

#include "egg2d.h"

static SDL_Surface *screenSurface = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static void  *assetsBuffer[13];
static char assetsIndex = 0;
static GLuint memoryUsage = 0;

#ifndef __APPLE__
#pragma pack(push, x1)
#pragma pack(1)
#endif

typedef struct {
    unsigned char IdSize,
            ImageType;
    unsigned short PaletteSize, PaletteEntryDepth;
    unsigned short X, Width,
            Y, Height;
    unsigned short MapType;
    unsigned char ColorDepth,
            Descriptor;
    unsigned short alpha;
    unsigned int r, g, b;
} PCM_HEADER;

#ifndef __APPLE__
#pragma pack(pop, x1)
#endif

/**
 * \private Initialize OpenGL viewport.
 */
static void initViewPort() {

//  Compute the aspect, horizontal or vertical scale, horizontal crop, and vertical crop.
    SDL_GetWindowSize(window, &g_actualWidth, &g_actualHeight);

    g_aspect = (int) ( (float)g_actualWidth / (float)g_actualHeight);
    g_cropH = 0;
    g_cropV = 0;

    if (g_aspect > g_targetAspect) {
        g_scale = ((float) g_actualWidth / (float)g_targetHeight);
        g_cropH = (int)((float)(g_actualWidth - (int)((float)g_targetWidth * g_scale)) * 0.5f);
    } else if (g_aspect < g_targetAspect) {
        g_scale = ((float)g_actualWidth / (float)g_targetWidth);
        g_cropV = (int)(((float)g_actualHeight - (float)g_targetHeight * g_scale) * 0.5f);
    } else { g_scale = (float) g_actualWidth / (float) g_targetWidth; }

    g_scaledWidth = (int)((float)g_targetWidth * g_scale);
    g_scaledHeight = (int)((float)g_targetHeight * g_scale);

    glViewport(g_cropH, g_cropV, g_scaledWidth, g_scaledHeight);
}

/**
 * Initialized OpenGL default 2D
 */
static int init2D() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if !defined(__ANDROID__)
//  Set Depth buffer to 1(d-Buffer)
    glClearDepth(1.0);
#endif
//  Disable Depth Testing so that our d-buffer works
    glDisable(GL_DEPTH_TEST);

#if !defined(__ANDROID__)
//  Compare each incoming pixel d value with the d value present in the depth buffer.
//  LEQUAL means than pixel is drawn if the incoming d value is less than
//  or equal to the stored d value.
    glDepthFunc(GL_LEQUAL);
#endif

//  Enable Texturing
    glEnable(GL_TEXTURE_2D);

//  Disable Backface culling
    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DITHER);

//  Clears both Window Buffer with Color, Depth, and Stencil Buffet set.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_GL_SwapWindow(window);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    return 0;
}

/**
 * \EGG ::Creates a new Window. \n
 * Defaults to SDL window.
 * @param title The title of the window.
 * @return -1: on error; 0 on success.
 */
int CreateWindow(const char *title) {
//  Initialize SDL (everything)
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
//  Prepare OpenGL 2.0 Context on SDL2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//  Sets Red, Green, Blue, Alpha to 8 bit, Depth size to 16.
//  With double buffering and accelerated visuals.
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

#if !defined(__ANDROID__)
//  Set swap interval to synchronized vertical re-trace.
    SDL_GL_SetSwapInterval(1);
#endif

    window = SDL_CreateWindow(title, 0, 0,
                              g_targetWidth, g_targetHeight,
                              SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
//      Do something on failure (to the callee).
        return -1;
    }

#if !defined(__ANDROID__)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "There was an error creating renderer (%s)\n", SDL_GetError());
        return -1;
    }
#endif


//  We will not actually need a context created, but we should create one
    SDL_GLContext gl = SDL_GL_CreateContext(window);

    if (gl == NULL) {
        fprintf(stderr, "There was an error creating GL_Context (%s)", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl);

#if !defined(__ANDROID__)
//  Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Cannot load GLAD\n");
        return -1;
    }
#endif

//  Initialize viewport
    initViewPort();

//  Print the OpenGL vendor, renderer, and version.
    //printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));

//    Get and save the created window surface.
#if !defined(__ANDROID__)
    screenSurface = SDL_GetWindowSurface(window);
    if (!screenSurface) {
        fprintf(stderr, "Could not get window surface: %s\n", SDL_GetError());
        return -1;
    }
#endif

    return init2D();
}

/**
 * \EGG ::Quit Program.\n
 * It will release the renderer, window, and additional loaded resources,
 * then gracefully quits the program.\n<br>
 *
 * Call this when terminating your ..::[Egg2D]::.. program.
 */
void EGG_Quit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    eggUnload();
    SDL_Quit();
}

// --- Shader Manager ---

/**
 * \EGG ::Compile (Vertex or Fragment) Shader.\n
 * Compiles a unit shader, then return its id, a graphics shader object identifier;
 * @type either a GL_VERTEX_SHADER or GL_FRAGMENT_SHADER;
 * @shaderSrc the relative path of shader source.
 * @returns the shader id. On error, it will return 0; otherwise, a positive id.
 */
GLuint eggCompileShader(GLenum type, const char *shaderSrc) {
    GLuint shader;
    GLint compiled;
//  Create new shader object.
    shader = glCreateShader(type);

    if (shader == 0) {
        fprintf(stderr, "could not create [%s] shader: %s\n",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", SDL_GetError());
        return 0;
    }
//  Set the shader source for compiling.
    glShaderSource(shader, 1, &shaderSrc, NULL);

//  Compile the shader
    glCompileShader(shader);

//  Check the compilation status.
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
//  On compilation error, print the error status info, deletes the shader, and then return 0.
    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 0) {
            char *infoLog = (char *) malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            fprintf(stderr, "%s", infoLog);

            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

/**
 * \EGG ::Creates and Compile Shader Program.\n
 * It will completely compile a shader program, using the specified
 * vertex and shader program id; use eggLoadVertShaderFile and eggLoadFragShaderFile.
 * @vertexShaderObj the vertex shader id;
 * @fragmentShaderObj the fragment shader id;
 * @returns the id of the program (located in the graphics memory);
 */
GLuint eggShaderCreateProgram(GLuint vertexShaderObj, GLuint fragmentShaderObj) {
    GLint linked; // link status

//  Combine the shaders into shader program. On error, print some error status, and return 0.
    GLuint shaderProgramObj = glCreateProgram();
    if (shaderProgramObj == 0) {
        fprintf(stderr, "There's an error creating shader shaderProgramObj.\n");
        GL_CHECK();
        return 0;
    }
//  Attach both vertex and fragment shader for program compiling.
    glAttachShader(shaderProgramObj, vertexShaderObj);
    glAttachShader(shaderProgramObj, fragmentShaderObj);
//  Link the shader program.
    glLinkProgram(shaderProgramObj);

//  Check the compilation status. On error, print some error status, then delete the shader program.
    glGetProgramiv(shaderProgramObj, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(shaderProgramObj, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > -1) {
            char *infoLog = (char *) malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(shaderProgramObj, infoLen, NULL, infoLog);
            fprintf(stderr, "Error linking shader-program [obj]\n%s\n", infoLog);

            free(infoLog);
        }
        glDeleteProgram(shaderProgramObj);
        return 0;
    }
    return shaderProgramObj;
}


/**
 * \EGG ::Load Vertex Shader File.\n
 * Loads the file, relative to the current path,
 * compiles, then return an `EggShader`
 * \EggShader a struct data type to contain
 * \type the shader type, either SHADER_FRAG or SHADER_VERT. Returns SHADER_VERT;
 * \id the shader object id (in graphics memory);
 * \src a TextResource to contain file's file pointer and file size;
 * \error the error is set when opening the file, either one of
 * SHADER_NO_ERROR, SHADER_READ_ERROR, or SHADER_COMPILE_ERROR.
 *
 */
struct EggShader eggLoadVertShaderFile(const char *relativePath) {
    //  Read vertex shader source.
    struct EggFileContext eggFile = eggFileOpen(NULL, relativePath);
    if (eggFile.size == -1) {
        return (struct EggShader) {
            .type = SHADER_VERT,
            .src = (struct TextResource) {
                .src = NULL,
                .size = -1
            },
            .error = SHADER_READ_ERROR
        };
    }
    long vertexBytesLen = (long) sizeof(char) * eggFile.size;
    char *vertexShaderSrc = (char *) malloc(vertexBytesLen);

    size_t bytesRead = eggFileRead(eggFile.filePointer, vertexBytesLen, vertexShaderSrc);
    vertexShaderSrc[bytesRead] = '\0';

    struct EggShader vertex = {
        .type = SHADER_VERT,
        .id = 0,
        .src = (struct TextResource) {
            .src = vertexShaderSrc,
            .size = (long)bytesRead
        },
        .error = SHADER_NO_ERROR
    };

//  Compile the vertex shader.
    GLuint vertexShaderObj = eggCompileShader(GL_VERTEX_SHADER, vertexShaderSrc);

    vertex.id = vertexShaderObj;
    eggFileClose(eggFile.filePointer);
//    free(vertexShaderSrc);

    if (vertexShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the vertex-shader [obj].\n");
#if defined(DEBUG)
        printf("%ld bytes\n", eggFile.size);
#endif
        GL_CHECK();
        free(vertexShaderSrc);
        vertex.src.src = NULL;
        vertex.src.size = -1;
        vertex.error = SHADER_COMPILE_ERROR;
//        SDL_Quit();

    }
    return vertex;
}
/**
 * \EGG ::Load Fragment Shader File.\n
 * Loads the file, relative to the current path,
 * compiles, then return an `EggShader`
 * \EggShader a struct data type to contain
 * \type the shader type, either SHADER_FRAG or SHADER_VERT. Returns SHADER_FRAG;
 * \id the shader object id (in graphics memory);
 * \src a TextResource to contain file's file pointer and file size;
 * \error the error is set when opening the file, either one of
 * SHADER_NO_ERROR, SHADER_READ_ERROR, or SHADER_COMPILE_ERROR.
 *
 */
struct EggShader eggLoadFragShaderFile(const char *relativePath) {

//  Read fragment shader source.
    EggFileContext eggFile = eggFileOpen(NULL, relativePath);

    if (eggFile.size == -1) {
        return (struct EggShader) {
            .type = SHADER_FRAG,
            .src = (struct TextResource) {
                .src = NULL,
                .size = -1
            },
            .error = SHADER_READ_ERROR
        };
    }

    long fragmentBytesLen = (long)sizeof(char) * eggFile.size;
    char *fragmentShaderSrc = (char *) malloc(fragmentBytesLen);

    size_t bytesRead = eggFileRead(eggFile.filePointer, fragmentBytesLen, fragmentShaderSrc);
    fragmentShaderSrc[bytesRead] = '\0';

    struct EggShader frag = {
            .type = SHADER_FRAG,
            .id = 0,
            .src = (struct TextResource) {
                .src = fragmentShaderSrc,
                .size = (long)bytesRead
            },
            .error = SHADER_NO_ERROR
    };

//  Compile the fragment shader.
    GLuint fragmentShaderObj = eggCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    frag.id = fragmentShaderObj;
    eggFileClose(eggFile.filePointer);

    if (fragmentShaderObj == 0) {
        fprintf(stderr,
                "There's an error compiling the fragment-shader [obj].\n");
#if defined(DEBUG)
        printf("%ld bytes\n", eggFile.size)
#endif
        GL_CHECK();
        free(fragmentShaderSrc);
        frag.src.src = NULL;
        frag.src.size = -1;
        frag.error = SHADER_COMPILE_ERROR;
    }
    return frag;
}

/**
 * \EGG ::Free/Release Shader.
 *
 * Release the EggShader object.
 * @obj the shader, either a vertex or fragment shader.
 */
void eggFreeShader(EggShader obj) {
    if (obj.src.src != NULL) {
        free((char *) obj.src.src);
    }
}


// --- LOGGING ---

/**
 * EggUtil Log message.
 * @param formatStr
 * @param ...
 */
EGG_API void eggLogMessage(const char *formatStr, ...) {
    va_list params;
    char buf[BUFSIZ];

    va_start(params, formatStr);
    vsprintf(buf, formatStr, params);

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "eggUtil", "%s", buf);
#else
    printf("%s", buf);
#endif

    va_end(params);
}

/**
 * \EGG:: File Open.\n
 * Opens a file, (on android, input-output context is use to load assets by the asset manager),
 * query its size, and then return an `EggFileContext`
 * @returns EggFileContext
 * \EggFileContext c-struct to contain return values for opening the file.
 * \filePointer the file pointer of opened file, on error, it sets to NULL pointer \n
 * \size the physical size of file, on error, it sets to -1 \n
 *
 *
 */
EGG_API struct EggFileContext eggFileOpen(void *ioContext, const char *fileName) {
    eggFile *pFile = NULL;

#ifdef ANDROID
    if (ioContext != NULL)
    {
        AAssetManager = *assetManager = (AAssetManager *) ioContext;
        pFile = AAssetManager_open(assetManager, fileName);
    }
#else
    pFile = fopen(fileName, "rb");
#endif
    if (pFile == NULL) {
        fprintf(stderr, "Unable to open file '%s", fileName);
        return (struct EggFileContext) {
            .filePointer = NULL,
            .size = -1
        };
    }
    fseek(pFile, 0, SEEK_END);
    const long file_size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    if (file_size == -1) {
        fprintf(stderr, "Unable to read file '%s'", fileName);
        fclose(pFile);
        return (struct EggFileContext) {
                .filePointer = NULL,
                .size = -1
        };
    }
    return (struct EggFileContext) {
        .filePointer = pFile,
        .size = file_size };
}

/**
 * \EGG ::File Close.\n
 *
 * Close the opened file.
 * @pFile the file pointer to close.
 */
EGG_API void eggFileClose(eggFile *pFile) {
    if (pFile != NULL) {
#ifdef ANDROID
        AAsset_close(pFile);
#else
        fclose(pFile);
        pFile = NULL;
#endif
    }
}

/**
 * \EGG ::File Read.\n
 *
 * Read file to buffer, then returns the read-size.
 * @pFile the opened file pointer to file to read.
 * @bytesToRead size limit to read.
 * @buffer a pointer to use upon reading the file.
 * @returns the read-size of file read.
 */
EGG_API size_t eggFileRead(eggFile *pFile, long bytesToRead, void *buffer) {
    unsigned long bytesRead = 0;

    if (pFile == NULL) {
        return bytesRead;
    }

#ifdef ANDROID
    bytesRead = AAsset_read(pFile, buffer, bytesToRead);
#else
    bytesRead = fread(buffer, bytesToRead, 1, pFile);
#endif

    return bytesRead == 0 || bytesRead == 1 ? bytesToRead : bytesRead;
}

/**
 * (!!) Only call when you have allocated memory to the heap!
 * It will deallocate all assets in the memory. So use it with caution!
 */
void acquireAssetToMemory(void *bytes, unsigned int size) {
    assetsBuffer[assetsIndex] = bytes;
    assetsIndex ++;
    memoryUsage += size;

    eggLogMessage("Using %d bytes of memory\n", memoryUsage);
}
/**
 * REL's Glow-Image.
 * <li><u>https://rel.phatcode.net</li>
 * @return OpenGL texture ID
 */
GLuint GetGlowImage() {

    static GLuint textureID = 0;

    if (textureID == 0) {

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        int width, height;
        char *bytes = eggLoadPCM(NULL, "./glow_image.pcm", &width, &height);

        if (bytes == NULL) {
            eggLogMessage("[GetGlowImage] unable to open glow image..\n");
            return 0;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }

    return textureID;
}

/**
 * \EGG ::Load PCM Image\n
 *
 * Loads an (unconventional) PCM image format.\n
 * PCM is a Pure Color Map, array data map for color map textures.
 */
EGG_API char *eggLoadPCM(void *ioContext, const char *fileName, int *width, int *height) {
    char *buffer;
    eggFile *fp;
    PCM_HEADER Header;
    size_t bytesRead;

    fp = eggFileOpen(ioContext, fileName).filePointer;

    if (fp == NULL) {
        eggLogMessage("[eggLoadPCM] FAILED to load: { %s }\n", fileName);
        return NULL;
    }

    bytesRead = eggFileRead(fp, sizeof(PCM_HEADER), &Header);

    *width = Header.Width;
    *height = Header.Height;

    if (Header.ColorDepth == 128) {
        int bytesToRead = (int)sizeof(char) * (*width) * (*height) * Header.ColorDepth / 8;

//      Allocate the image data buffer
        buffer = (char *) malloc(bytesToRead);

        if (buffer) {
            bytesRead = eggFileRead(fp, bytesToRead, buffer);
            acquireAssetToMemory(buffer, bytesRead);
            eggFileClose(fp);

            return (buffer);
        }
    }
    return (NULL);
}

/**
 * \private ::Frame Buffer to Texture.
 * @returns the allocated texture of snapshot to framebuffer.
 */
static char* frameBufferToTexture(GLuint framebuffer, GLsizei width, GLsizei height) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    int texLength = width * height * 4;
    char *texture = (char *) malloc(texLength);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, texture);
    return texture;
}

/**
 * \EGG ::Get Shader Uniforms. \n
 *
 * Query the Shader Uniforms (to print all those available uniforms).
 * @returns the total count of shader uniforms.
 */
GLint eggGetUniforms(GLuint program) {
    GLint params;
    GL_CHECK(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &params));

    for (int i = 0; i < params; i++) {
        GLsizei len;
        GLchar name[64];
        glGetActiveUniformName(program, i, 64, &len, name);
        SDL_Log("Uniform '%s'; ", name);
    }

    return params;
}

/**
 * \EGG ::Unload Resources.\n
 *
 * Try deallocating managed memory resources.
 */
void eggUnload() {
    eggLogMessage("Freeing up resources..");
    for (int ai = 0; ai < assetsIndex; ai++) {
        if (assetsBuffer[ai] != NULL) {
            free(assetsBuffer[ai]);
            assetsBuffer[ai] = NULL;
        }
    }
    eggLogMessage(" %d KBs freed!\n Bye!\n", memoryUsage / 1024);
}

/**
 * \EGG2D_API ::Set Background Color.
 */
void setBackgroundColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
}

/**
 * \EGG2D_API Clears the depth, stencil, and color of the screen.
 */
void ClearScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/**
 * \EGG2D_API Updates and synchronize the window.
 */
void UpdateWindow() {
    SDL_GL_SwapWindow(window);
}