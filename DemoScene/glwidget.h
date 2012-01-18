#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <stdio.h>
#include <object.h>
#include <QListWidgetItem>
#include <model.h>
#include <QGLShader>
#include <QGLShaderProgram>
#include <QGLBuffer>
#include <QtOpenGL>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    Model loadOBJ(int textureUnit, QString fileName);

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

    void xLookAtChanged(double value);
    void yLookAtChanged(double value);
    void zLookAtChanged(double value);
    void loadNewModel(int textureUnit, QString model);
    // slot to handle timer updates
    void timerExpired();
signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void modelLoaded(Model m);
protected:
    // overloaded initGL function
    void initializeGL();
    // overloaded paintGL function
    void paintGL();
    //overloaded resizeGL function
    void resizeGL(int width, int height);

    // handle mouse inputs
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    void setMaterial(Model model, QString name);
    void loadMaterial(Model model, QString name);
    void project();
    int buildSkyBox(float scale);
    unsigned int loadTexture(QString fileName);
    Model getModelByName(QString name);

    int rotation[3];    // model rotation
    double lookAt[3];   // camera look at vector
    GLfloat lightPos[4];    // light position
    double modelLocation[3]; // location of the model

    double toRad(int deg);
    int modelDisplayList; // currently loaded model
    int numMaterial;    // total number of loaded materials
    bool perspective;   // in perspective mode?

    QPoint lastPos;     // last mouse position
    QColor qtPurple;    // background color

    QList<Model> loadedModels; // loaded models
    Model terrainModel;         // terrain model being displayed
    Model skyBoxModel;          // sky box model being displayed

    // New load OBJ
   // QList<material>* mtlinfo;   // all materials associated with a model

    // shaders
    QGLShaderProgram program;   // load and use shaders

    QTimer *t;
    double time;

    int theta, phi;

    GLfloat waveAmp[8];
    GLfloat wavelength[8];
    GLfloat waveSpeed[8];
    QVector2D direction[8];

};

#endif // GLWIDGET_H
