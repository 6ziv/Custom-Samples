#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "menu.hpp"
#include "musicplayer.hpp"
class GLWidget :
        public QOpenGLWidget
{
public:
    GLWidget(int w,int h);
    void initializeGL()override;
    void resizeGL(int width, int height)override;
    void paintGL()override;
    void testMouse();
    void clear();
    bool nativeEvent(const QByteArray &,void*,long*)override;
private:
    Menu menu;
    bool is_dragging;
    int oldx,oldy;
    void mousePressEvent(QMouseEvent*e)override;
    void mouseReleaseEvent(QMouseEvent*e)override;
    void mouseMoveEvent(QMouseEvent*e)override;
};

#endif // GLWIDGET_H
