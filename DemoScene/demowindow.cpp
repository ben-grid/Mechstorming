#include "demowindow.h"
#include "ui_demowindow.h"

DemoWindow::DemoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DemoWindow)
{
    ui->setupUi(this);
    ui->GlLayout->addWidget(&gl);
}

DemoWindow::~DemoWindow()
{
    delete ui;
}
