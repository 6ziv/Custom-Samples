#ifndef APPUTILS_H
#define APPUTILS_H
#include <CubismFramework.hpp>
#include <string>
#include <iostream>
class AppUtils
{
public:
    typedef std::vector<std::byte> FileContent;
    static bool readFileContent(const std::string filePath, std::vector<std::byte>& result);
    inline static Csm::csmFloat32 GetDeltaTime(){
        return static_cast<Csm::csmFloat32>(s_deltaTime);
    }
    inline static void UpdateTime(){
        s_currentFrame = GetTickCount64();
        s_deltaTime = (s_currentFrame - s_lastFrame)/1000.0;
        s_lastFrame = s_currentFrame;
    }
    inline static void Log(const Csm::csmChar* message){
        std::cout<<message<<std::endl;
    }
private:
    inline static unsigned long long s_currentFrame=0;
    inline static unsigned long long s_lastFrame=0;
    inline static long double s_deltaTime=0.0;
};
#endif
