#ifndef DEMOWINDOW_H
#define DEMOWINDOW_H

#include <QDialog>
#include <glwidget.h>

namespace Ui {
    class DemoWindow;
}

class DemoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DemoWindow(QWidget *parent = 0);
    ~DemoWindow();

private:
    Ui::DemoWindow *ui;
    GLWidget gl;
};

#endif // DEMOWINDOW_H
