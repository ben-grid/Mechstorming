#ifndef OBJECT_H
#define OBJECT_H

#include <QString>
#include <QtOpenGL>

typedef struct{
    QString name;                 //  Material name
    float Ka[4],Kd[4],Ks[4],Ns; //  Colors and shininess
    float d;                    //  Transparency
    int map;                    //  Texture
} material;

typedef struct{
    float x,y,z;
} vertex;

typedef struct{
    float s,t;
}tex;

typedef struct{
    float x,y,z;
} normal;

typedef struct{
    GLfloat x,y,z;
    GLfloat nx,ny,nz;
    GLfloat s,t;
}vert;


typedef struct{
    float Emission[4];
    float Ambient[4];
    float Diffuse[4] ;
    float Specular[4];
    float Shinyness[1];
} materialParams;

#endif // OBJECT_H
