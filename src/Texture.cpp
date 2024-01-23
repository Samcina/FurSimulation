#include "Texture.h"

Texture::Texture() {
    width = 0;
    height = 0;
    textureID = 0;
    type = 0;
    imageData = NULL;
    bpp = 0;
}

Texture::Texture(const char* filename) {
    createTexture(filename);
}

Texture::~Texture() { }

int Texture::loadUncompressedTGA(FILE* TGAfile) {
    GLubyte temp;
    GLuint cswap;
    TGA tga;

    if (fread(tga.header, sizeof(tga.header), 1, TGAfile) == 0) {
        fprintf(stderr, "Could not read info header.\n");
        if (TGAfile != NULL) {
            fclose(TGAfile);
        }
        return GL_FALSE;
    }

    this->width = tga.header[1] * 256 + tga.header[0];
    this->height = tga.header[3] * 256 + tga.header[2];
    this->bpp = tga.header[4];
    tga.width = this->width;
    tga.height = this->height;
    tga.bpp = this->bpp;

    if ((this->width <= 0) || (this->height <= 0) || ((this->bpp != 24) && (this->bpp != 32))) {
        fprintf(stderr, "Invalid texture information.\n");
        if (TGAfile != NULL) {
            fclose(TGAfile);
        }
        return GL_FALSE;
    }

    if (bpp == 24) {
        this->type = GL_RGB;
        printf("Texture type is GL_RGB\n");
    }
    else {
        this->type = GL_RGBA;
        printf("Texture type is GL_RGBA\n");
    }

    tga.bytesPerPixel = (tga.bpp / 8);
    tga.imageSize = (tga.bytesPerPixel * tga.width * tga.height);
    this->imageData = new GLubyte[tga.imageSize];

    if (this->imageData == NULL) {
        fprintf(stderr, "Could not allocate memory for image.\n");
        fclose(TGAfile);
        return GL_FALSE;
    }

    if (fread(this->imageData, 1, tga.imageSize, TGAfile) != tga.imageSize) {
        fprintf(stderr, "Could not read image data.\n");
        if (this->imageData != NULL) {
            delete[] this->imageData;
        }
        fclose(TGAfile);
        return GL_FALSE;
    }

    for (cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel) {
        temp = this->imageData[cswap];
        this->imageData[cswap] = this->imageData[cswap + 2];
        this->imageData[cswap + 2] = temp;
    }

    fclose(TGAfile);
    return GL_TRUE;
}

int Texture::loadTGA(const char* filename) {
    FILE* TGAfile;
    TGAHeader tgaheader;

    GLubyte uTGAcompare[12] = { 0,0,2, 0,0,0,0,0,0,0,0,0 };
    GLubyte cTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };

    TGAfile = fopen(filename, "rb");

    if (TGAfile == NULL) {
        fprintf(stderr, "Could not open texture file.\n");
        return GL_FALSE;
    }

    if (fread(&tgaheader, sizeof(TGAHeader), 1, TGAfile) == 0) {
        fprintf(stderr, "Could not read file header.\n");
        if (TGAfile != NULL) {
            fclose(TGAfile);
        }
        return GL_FALSE;
    }

    if (memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {
        this->loadUncompressedTGA(TGAfile);
    }
    else if (memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0) {
        fprintf(stderr, "RLE compressed TGA files are not supported.\n");
        fclose(TGAfile);
        return GL_FALSE;
    }
    else {
        fprintf(stderr, "Unsupported image file format.\n");
        fclose(TGAfile);
        return GL_FALSE;
    }
    return GL_TRUE;
}

void Texture::createTexture(const char* filename) {
    this->loadTGA(filename);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &(this->textureID));
    glBindTexture(GL_TEXTURE_2D, this->textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, this->type, GL_UNSIGNED_BYTE, this->imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] this->imageData;
}
