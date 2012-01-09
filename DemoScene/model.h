#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QList>
#include <QtOpenGL>
#include <object.h>
#include <stdlib.h>
#include <stdio.h>

class Model
{
public:
    Model();
    Model(QString modelName);
    Model(QString modelName, int displayID);
    ~Model();

    void setVertexData(QList<vertex>* v);
    void setNormalData(QList<vertex>* n);
    void setTextureData(QList<tex>* t);
    void setDisplayID(int newID);
    void setPath(QString newPath);
    void addMaterial(material m);
    void setIndexData(QList<GLuint>* f);
    void addFace(int face);

    QList<vertex>* getNormals() {return normals;}
    QList<vertex>* getVerts() {return verts;}
    QList<tex>* getTexCoords() {return texcoords;}
    QList<material>* getMaterials() { return mats;}
    QList<GLuint>* getFaces() { return indc;}
    GLuint* getFacesAsArray();
    vertex getVertex(int index);
    vertex getNormal(int index);
    tex getTexCoord(int index);


    int getDisplayList() { return displayListID;}

    QString getPath() { return this->path; }
    QString name;

private:

    int displayListID;

    QList<vertex>* normals;
    QList<tex>* texcoords;
    QList<vertex>* verts;
    QList<GLuint>* indc;
    QList<material>* mats;
    QString path;
};

#endif // MODEL_H
