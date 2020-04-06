#ifndef OPENGLHELPER_H
#define OPENGLHELPER_H
#include <QOpenGLFunctions>
class OpenGLHelper{
public:
    inline static QOpenGLFunctions* get(){
        if(instance==nullptr)
            instance=new QOpenGLFunctions;
        return instance;
    }
    inline static void release(){
        if(instance!=nullptr)
            delete instance;
        instance=nullptr;
    }
private:
    inline OpenGLHelper(){}
    inline ~OpenGLHelper(){}
    inline static QOpenGLFunctions* instance=nullptr;
};

#endif // OPENGLHELPER_H
