#include "app.hpp"
#include <iostream>
#include "apputils.hpp"
#include "config.hpp"
#include "textureloader.hpp"
#include "openglhelper.hpp"
#include "model.hpp"
using namespace Csm;
using namespace std;
bool App::Initialize(int w,int h){
    _window =new GLWidget(w,h);
    if (_window == nullptr)
        return false;
    InitializeCubism();
    initialized=true;
    return true;
}
void App::Run()
{
    if(!initialized)return;
    _window->clear();
    AppUtils::UpdateTime();
    _model->Update();
    _model->Draw();
}

void App::InitializeCubism()
{

    _cubismOption.LogFunction = AppUtils::Log;
    _cubismOption.LoggingLevel = Config::CubismLoggingLevel;
    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
    CubismFramework::Initialize();

    std::string modelPath = std::string(Config::ResourcesPath) + Config::ModelDir + "/";
    std::string modelJsonName = Config::ModelDir;
    modelJsonName += ".model3.json";
    _model=new Model();
    _model->LoadAssets(modelPath.c_str(), modelJsonName.c_str());

    AppUtils::UpdateTime();
}


GLuint App::CreateShader()
{
    GLuint vertexShaderId = OpenGLHelper::get()->glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShader =
        "#version 120\n"
        "attribute vec3 position;"
        "attribute vec2 uv;"
        "varying vec2 vuv;"
        "void main(void){"
        "    gl_Position = vec4(position, 1.0);"
        "    vuv = uv;"
        "}";
    OpenGLHelper::get()->glShaderSource(vertexShaderId, 1, &vertexShader, NULL);
    OpenGLHelper::get()->glCompileShader(vertexShaderId);
    if(!CheckShader(vertexShaderId))
    {
        return 0;
    }

    GLuint fragmentShaderId = OpenGLHelper::get()->glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShader =
        "#version 120\n"
        "varying vec2 vuv;"
        "uniform sampler2D texture;"
        "uniform vec4 baseColor;"
        "void main(void){"
        "    gl_FragColor = texture2D(texture, vuv) * baseColor;"
        "}";
    OpenGLHelper::get()->glShaderSource(fragmentShaderId, 1, &fragmentShader, NULL);
    OpenGLHelper::get()->glCompileShader(fragmentShaderId);
    if (!CheckShader(fragmentShaderId))
    {
        return 0;
    }

    GLuint programId = OpenGLHelper::get()->glCreateProgram();
    OpenGLHelper::get()->glAttachShader(programId, vertexShaderId);
    OpenGLHelper::get()->glAttachShader(programId, fragmentShaderId);

    OpenGLHelper::get()->glLinkProgram(programId);

    OpenGLHelper::get()->glUseProgram(programId);

    return programId;
}

bool App::CheckShader(GLuint shaderId)
{
    GLint status;
    GLint logLength;
    OpenGLHelper::get()->glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        OpenGLHelper::get()->glGetShaderInfoLog(shaderId, logLength, &logLength, log);
        CubismLogError("Shader compile log: %s", log);
        CSM_FREE(log);
    }
    OpenGLHelper::get()->glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        OpenGLHelper::get()->glDeleteShader(shaderId);
        return false;
    }
    return true;
}
