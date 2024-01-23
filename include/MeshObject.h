#ifndef OPENGL_TEMPLATE_MESHOBJECT_H
#define OPENGL_TEMPLATE_MESHOBJECT_H

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI (3.14159265359)
#endif

class MeshObject {

public:

    MeshObject();
    ~MeshObject();
    void clean();
    void createSphere(float radius, int segments);
    void readOBJ(const char* filename);
    void print();
    void printInfo();

    GLfloat* getVertexArray() const{
        return vertexarray;
    }

    int getNoOfVertices() const{
        return nverts;
    }

    void render(bool tesselationShadersUsed);

private:
    GLuint vao;
    int nverts;
    int ntris;
    GLuint vertexbuffer;
    GLuint indexbuffer;
    GLfloat *vertexarray;
    GLuint *indexarray;
    void printError(const char *errtype, const char *errmsg);
};


#endif