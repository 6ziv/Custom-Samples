#pragma once

#include "allocator.hpp"
#include "glwidget.hpp"
#include "openglhelper.hpp"
#include "model.hpp"

class App
{
public:
    inline static App* GetInstance(){
        if(instance==nullptr)
            instance=new App;
        return instance;
    }
    bool Initialize(int w,int h);
    void Run();
    GLuint CreateShader();
    inline Model* GetModel(){
        return _model;
    }
    inline GLWidget* GetWindow() {
        return _window;
    }
    inline bool IsReady(){return initialized;};
    inline void dowork(){
        _window->update();
        _window->testMouse();
    }
private:
    inline App(){}
    inline ~App(){
        _window->deleteLater();
        OpenGLHelper::release();
        Csm::CubismFramework::Dispose();
        _model->DeleteRenderer();
        delete _model;
    }
    bool initialized=false;
    void InitializeCubism();
    bool CheckShader(GLuint shaderId);

    Allocator _cubismAllocator;
    Csm::CubismFramework::Option _cubismOption;
    GLWidget* _window=nullptr;

    inline static App* instance=nullptr;
    Model* _model;
};
