#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>

class Texture {
public:
    GLuint width;
    GLuint height;
    GLuint textureID;
    GLuint type;

    Texture();
    Texture(const char* filename);
    ~Texture();

    void createTexture(const char* filename);

private:
    GLubyte* imageData;
    GLuint bpp;

    int loadUncompressedTGA(FILE* tgafile);
    int loadTGA(const char* filename);
};

typedef struct {
    GLubyte Header[12];
} TGAHeader;

typedef struct {
    GLubyte header[6];
    GLuint bytesPerPixel;
    GLuint imageSize;
    GLuint type;
    GLuint height;
    GLuint width;
    GLuint bpp;
} TGA;
