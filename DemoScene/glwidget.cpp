#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
 : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    // set default look at
    lookAt[0]=0;
    lookAt[1]=0;
    lookAt[2]=10;

    // set initial light position
    lightPos = { 0,0,0,1 };

    // load the model and move it down 100 units
    modelLocation[0] = 0;
    modelLocation[1] = -100;
    modelLocation[2] = 0;


    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

    // no models loaded yet so set it to a invalid id
    modelDisplayList = -1;

    numMaterial = 0;
    // create new timer to handle animation
    t = new QTimer();

    // connect the timers timeout signal to the timer expired slot
    connect(t, SIGNAL(timeout()), this, SLOT(timerExpired()));
    // start the timer with a 30ms timout (30ms ~ 30Hz)
    t->start(30);
    // steal all keyboard inputs
    this->grabKeyboard();
    theta = 180;
    phi = 0;


    // update the GL context
    updateGL();

    // go load the terain
    loadNewModel(GL_TEXTURE0, "./models/terrain1.obj");

    this->terrainModel = getModelByName("terrain1.obj");

    // activate all the textures
    glActiveTexture(GL_TEXTURE1);
    loadTexture("./models/Grass.jpg");
    glActiveTexture(GL_TEXTURE2);
    loadTexture("./models/WoodChips.jpg");
    glActiveTexture(GL_TEXTURE3);
    loadTexture("./models/SkyBox.jpg");

    // build the skybox
    this->skyBoxModel.setDisplayID( buildSkyBox(490));

    // activate the skybox texture
    glActiveTexture(GL_TEXTURE0);

    // load all shaders
    program.removeAllShaders();
    program.addShaderFromSourceFile(QGLShader::Vertex, "./shaders/perpixelphong.vert");
    program.addShaderFromSourceFile(QGLShader::Fragment, "./shaders/perpixelphong.frag");

    // compile and bind the shader
    program.bind();
    // update all uniform values
    program.setUniformValue("heightMap",0);
    program.setUniformValue("baseMap1", 1);
    program.setUniformValue("baseMap2", 2);
    program.setUniformValue("skybox", 3);
    program.setUniformValue("mode", int(0));
    program.setUniformValue("envMap", 3);
    program.setUniformValue("numWaves", int(4));


    // set the parameter for the sum of sins equation
    waveAmp = {1.5,1.5,1,2, 10,10,10,10};
    wavelength= {50,30,50,30, 10,10,10,10};
    waveSpeed = {10,20,10,10, 10,10,10,10};
    direction[0] = QVector2D(-.2,.5);
    direction[1] = QVector2D(.5, -.3);
    direction[2] = QVector2D(0,1);
    direction[3] = QVector2D(-.1, -.8);
    program.setUniformValueArray("direction", direction, 4);
    program.setUniformValueArray("amplitude", waveAmp, 8,1);
    program.setUniformValueArray("wavelength", wavelength, 8,1);
    program.setUniformValueArray("speed", waveSpeed, 8,1);

}

GLWidget::~GLWidget()
{
    // when the widget distructs stop capturing all keyboard events
    this->releaseKeyboard();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != rotation[0]) {
        rotation[0]= angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != rotation[1]) {
        rotation[1] = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != rotation[2]) {
        rotation[2] = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::initializeGL()
{
    qglClearColor(qtPurple.dark());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_NORMALIZE);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    float Ambient[4]   = {0.3,0.3,0.3,1.0};
    float Diffuse[4]   = {1.0,1.0,1.0,1.0};
    float Specular[4]  = {1.0,1.0,1.0,1.0};

    glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);

    int numTextureUnits;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &numTextureUnits);

    int numBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &numBuffers);

    qDebug(QString("Number of available texture units: %1").arg(numTextureUnits).toAscii());
    qDebug(QString("Number of buffers: %1").arg(numBuffers).toAscii());
}

void GLWidget::paintGL()
{
    // clear the depth and color buffors
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // reset gl's matrix stack.
    glLoadIdentity();

    // set the projection matrix
    project();
    float Specular[4]  = {1.0,1.0,1.0,1.0};
    float Emission[4]  = {0.0,0.0,0.0,1.0};
    float Shinyness[4] = {1};

    if(terrainModel.getDisplayList() >= 0){
        glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color

        glPushMatrix();
        glTranslatef(modelLocation[0],modelLocation[1],modelLocation[2]);

        glRotatef(-90, 1.0, 0.0, 0.0);
        program.setUniformValue("mode", 0);
        glCallList(terrainModel.getDisplayList());

        program.setUniformValue("mode", 1);
        glCallList(skyBoxModel.getDisplayList());

        glPushMatrix();
        glTranslatef(0,0,33);
        program.setUniformValue("mode", 2);
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,Shinyness);
        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
        glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

        glColor4f(0.6, 0.8, 1.0, .1);
        glCallList(terrainModel.getDisplayList());
        glDisable(GL_BLEND);
        glPopMatrix();
        glPopMatrix();

    }
    glFlush();
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    project();
}


void GLWidget::timerExpired(){
    time = fmod(time+.03, 3000000);

    if(program.isLinked()){
        program.setUniformValue("time", (GLfloat)time);
        updateGL();
    }
}

Model GLWidget::getModelByName(QString name){
    Model m;
    foreach(m , loadedModels ){
        if(m.name == name){
            return m;
        }
    }
    m.name = "empty";
    return m;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(rotation[0] + 8 * dy);
        setYRotation(rotation[1] + 8 * dx);

        theta += dx/2;
        phi += dy/2;

        theta %= 360;
        phi %=360;

    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(rotation[0] + 8 * dy);
        setZRotation(rotation[2] + 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event){

    if(event->orientation() == Qt::Vertical){
        modelLocation[1] += event->delta()*.3;

    }
    updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent* event){
    if(event->key() == Qt::Key_W){
        modelLocation[2] +=5;
    }
    else if(event->key() == Qt::Key_A){
        modelLocation[0] +=  5;
    }
    else if(event->key() == Qt::Key_S){
        modelLocation[2] -=5;
    }
    else if(event->key() == Qt::Key_D){
        modelLocation[0] -=5;
    }
    updateGL();
}

void GLWidget::project(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60,(float)this->geometry().width()/(float)this->geometry().height(),1,10000);

    double Ex = -2*sin(toRad(theta))*cos(toRad(phi));
    double Ey = +2        *sin(toRad(phi));
    double Ez = +2*cos(toRad(theta))*cos(toRad(phi));
    gluLookAt( 0,0, 0 , Ex,Ey,Ez , 0,cos(toRad(phi)),0);

   // gluLookAt(lookAt[0], lookAt[1], ltimerExpiredookAt[2], 0, 0, 0, 0, 1, 0);

    glMatrixMode(GL_MODELVIEW);
   // updateGL();
}


double GLWidget::toRad(int deg){
    return double(deg)*(3.141592653/180.0);
}

void GLWidget::xLookAtChanged(double value){
    lookAt[0] = value;
    project();
}

void GLWidget::yLookAtChanged(double value){
    lookAt[1] = value;
    project();

}
void GLWidget::zLookAtChanged(double value){
    lookAt[2] = value;
    project();
}

int GLWidget::buildSkyBox(float scale){
    int displayList = glGenLists(1); //  Start new displaylist
    glNewList(displayList, GL_COMPILE);
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_QUADS);  // Front
    glNormal3f( 0, 0,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .25);
    glVertex3f(-scale,-scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .75);
    glVertex3f(+scale,-scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .75);
    glVertex3f(+scale,+scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .25);
    glVertex3f(-scale,+scale,+scale);
    glEnd();


    glBegin(GL_QUADS);    //  Right
    glNormal3f(+scale, 0, 0);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .75);
    glVertex3f(+scale,-scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .25, 1);
    glVertex3f(+scale,-scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, .75, 1);
    glVertex3f(+scale,+scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .75);
    glVertex3f(+scale,+scale,+scale);
    glEnd();

    glBegin(GL_QUADS);    //  Left
    glNormal3f(-scale, 0, 0);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .0);
    glVertex3f(-scale,-scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .25);
    glVertex3f(-scale,-scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .25);
    glVertex3f(-scale,+scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .0);
    glVertex3f(-scale,+scale,0);
    glEnd();

    glBegin(GL_QUADS);    //  Top
    glNormal3f( 0,+scale, 0);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .25);
    glVertex3f(-scale,+scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .75, .75);
    glVertex3f(+scale,+scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, 1, .75);
    glVertex3f(+scale,+scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, 1, .25);
    glVertex3f(-scale,+scale,0);
    glEnd();

    glBegin(GL_QUADS);    //  Bottom
    glNormal3f( 0,-scale, 0);
    glMultiTexCoord2f(GL_TEXTURE3, 0, .25);
    glVertex3f(-scale,-scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, 0, .75);
    glVertex3f(+scale,-scale,0);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .75);
    glVertex3f(+scale,-scale,+scale);
    glMultiTexCoord2f(GL_TEXTURE3, .25, .25);
    glVertex3f(-scale,-scale,+scale);
    glEnd();
    glPopMatrix();
    glEnable(GL_CULL_FACE);
     glEndList(); // end display list

     return displayList;
}


void GLWidget::loadNewModel(int textureUnit, QString model){
    Model m;
    foreach(m , loadedModels ){
        if(m.name == QFileInfo(model).baseName()){
            return;
        }
    }
    m = loadOBJ(textureUnit, model);
    loadedModels.append(m);
    qDebug(QString("loaded model: %1").arg(m.name).toAscii());
    updateGL();
}

Model GLWidget::loadOBJ(int textureUnit, QString fileName){

    // store all the model info
    QList<vertex>* normals = new QList<vertex>();
    QList<tex>* texcoords = new QList<tex>();
    QList<vertex>* verts = new QList<vertex>();

    Model m(QFileInfo(fileName).baseName());
    m.setPath(fileName);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return m;

    int displayList = glGenLists(1); //  Start new displaylist
    glNewList(displayList, GL_COMPILE);
    glPushAttrib(GL_TEXTURE_BIT); //  Push attributes for textures

    QTextStream text(&file);
    QString line;
    QStringList list;
    while(!text.atEnd()){
        line = text.readLine();
        line = line.trimmed();
       //qDebug(line.toAscii());
        list = line.split(" ");

        if(list[0] == "v"){
            vertex v;
            v.x = list[1].toFloat();
            v.y = list[2].toFloat();
            v.z = list[3].toFloat();
            verts->append(v);
        }
        else if(list[0] == "vn"){
            vertex n;
            n.x = list[1].toFloat();
            n.y = list[2].toFloat();
            n.z = list[3].toFloat();
            normals->append(n);
        }
        else if(list[0] == "vt"){
            tex t;
            t.s = list[1].toFloat();
            t.t = list[2].toFloat();
            texcoords->append(t);
        }
        else if(list[0] == "f"){
            QStringList triple;
            vertex vt;
            vertex vn;
            tex tx;
            glBegin(GL_POLYGON);
            for(int i=1; i< list.size(); i++){
                triple = list.at(i).split("/");

                if(triple.size() == 1){
                    // Just a vertex
                    vt = verts->at(triple[0].toInt()-1);
                    glVertex3f(vt.x, vt.y, vt.z);
                }

                if(triple.size() > 1 && triple[1] != ""){
                    tx = texcoords->at(triple[1].toInt()-1);
                    glTexCoord2f( tx.s, tx.t);
                }
                if(triple.size() > 1 && triple[2] != ""){
                    vn = normals->at(triple[2].toInt()-1);
                    glNormal3f(vn.x, vn.y, vn.z);
                }
                if(triple.size() > 1 && triple[0] != ""){
                   vt = verts->at(triple[0].toInt()-1);
                   glVertex3f(vt.x, vt.y, vt.z);
                }
            }
            glEnd();
        }
        else if(list[0] == "mtllib"){
            glActiveTexture(textureUnit);
            loadMaterial(m, list[1]);
            glActiveTexture(GL_TEXTURE0);
        }
        else if(list[0] == "usemtl"){
            setMaterial(m, list[1]);
        }
    }
    glPopAttrib();
    glEndList(); // end display list
    file.close();
    qDebug(QString("Setting display list to:%1").arg(displayList).toAscii());
    m.setDisplayID(displayList);

    return m;
}

void GLWidget::loadMaterial(Model model, QString name){
    // store all the model info
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug("Error opening material file");
        return;
    }

    QTextStream text(&file);
    QString line;
    QStringList list;
    material m; // new material;
    bool openMaterial = false;

    while(!text.atEnd()){
        line = text.readLine().trimmed();
        list = line.split(" ");

        if(list[0] == "newmtl"){ // found a new material

            m.name = list[1];
            m.Ka[0] = m.Ka[1] = m.Ka[2] = 0;   m.Ka[3] = 1;
            m.Kd[0] = m.Kd[1] = m.Kd[2] = 0;   m.Kd[3] = 1;
            m.Ks[0] = m.Ks[1] = m.Ks[2] = 0;   m.Ks[3] = 1;
            m.Ns  = 0;
            m.d   = 0;
            m.map = 0;
            openMaterial = true;
        }
        else if(list[0] == "Ka"){
            m.Ka[0] = list[1].toFloat();
            m.Ka[1] = list[2].toFloat();
            m.Ka[2] = list[3].toFloat();
            if(list.size() == 5)
                m.Ka[3] = list[4].toFloat();
        }
        else if(list[0] == "Kd"){
            m.Kd[0] = list[1].toFloat();
            m.Kd[1] = list[2].toFloat();
            m.Kd[2] = list[3].toFloat();
            if(list.size() == 5)
                m.Kd[3] = list[4].toFloat();
        }
        else if(list[0] == "Ks"){
            m.Ks[0] = list[1].toFloat();
            m.Ks[1] = list[2].toFloat();
            m.Ks[2] = list[3].toFloat();
            if(list.size() == 5)
                m.Ks[3] = list[4].toFloat();
        }
        else if(list[0] == "map_Kd"){
            m.map = loadTexture(list[1]);

        }
        else if(line == ""){
            if(openMaterial){
                model.addMaterial(m);
            }
        }
    }

    file.close();
}

void GLWidget::setMaterial(Model model, QString name){

    QList<material>* modelmats = model.getMaterials();
    for(int i =0; i<modelmats->size(); i++){

        if (modelmats->at(i).name == name)
        {
            material m = modelmats->at(i);
         //  Set material colors
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT  ,m.Ka);
            glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE  ,m.Kd);
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR ,m.Ks);
            glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&m.Ns);
        //  Bind texture if specified
            if (m.map)
            {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,m.map);
            }
            else
                    glDisable(GL_TEXTURE_2D);
            return;
        }
    }
    qDebug(QString("material %1 not found").arg(name).toAscii());
}


unsigned int GLWidget::loadTexture(QString fileName){
    // We need to do all the texture loading in the GL widget.
    // normal texture loading causes artifacts for a unknown reason.

    QImage t;
    QImage b;
    unsigned int texture;
    b.load(fileName);
    t = QGLWidget::convertToGLFormat( b );

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    return texture;
}
