#include "model.h"

Model::Model()
{
    this->displayListID = -1;
    this->texcoords = NULL;
    this->verts = NULL;
    this->normals = NULL;
    this->mats = new QList<material>();
    this->indc = new QList<GLuint>();
}

Model::Model(QString modelName){
    this->displayListID = -1;
    this->name = modelName;
    this->texcoords = NULL;
    this->verts = NULL;
    this->normals = NULL;
    this->mats = new QList<material>();
    this->indc = new QList<GLuint>();
}

Model::Model(QString modelName, int displayID){
    this->displayListID = -1;
    this->name = modelName;
    this->displayListID = displayID;
    this->texcoords = NULL;
    this->verts = NULL;
    this->normals = NULL;
    this->mats = new QList<material>();
    this->indc = new QList<GLuint>();
}

Model::~Model(){

}

GLuint* Model::getFacesAsArray(){
    GLuint* retval = (GLuint*)malloc(indc->size()*3*sizeof(GLuint));
    for(int i=0; i< indc->size(); i++){
        retval[i] = indc->at(i);

    }
    return retval;
}


vertex Model::getVertex(int index){
    if(index <= verts->size()){
        return verts->at(index);
    }
    else{
        qDebug(QString("Could not get vertex at index: %1").arg(index).toAscii());
    }
}

vertex Model::getNormal(int index){
    if(index <= normals->size()){
        return normals->at(index);
    }
    else{
        qDebug(QString("Could not get normal at index: %1").arg(index).toAscii());
    }
}

tex Model::getTexCoord(int index){
    if(index <= normals->size()){
        return texcoords->at(index);
    }
    else{
        qDebug(QString("Could not get normal at index: %1").arg(index).toAscii());
    }
}

void Model::addFace(int face){
    this->indc->append(face);
}

void Model::setVertexData(QList<vertex>* v){
    this->verts = v;
}

void Model::setNormalData(QList<vertex>* n){
    this->normals = n;
}

void Model::setTextureData(QList<tex>* t){
    this->texcoords = t;
}

void Model::setDisplayID(int newID){
    this->displayListID = newID;
}

void Model::setPath(QString newPath){
    this->path= newPath;
}

void Model::setIndexData(QList<GLuint> *f){
    indc = f;
}

void Model::addMaterial(material m){
    if(this->mats != NULL){
        mats->append(m);
        return;
    }
    else{
        qDebug(QString("unable to add material %1, to model %2").arg(m.name).arg(this->name).toAscii());
    }
}


