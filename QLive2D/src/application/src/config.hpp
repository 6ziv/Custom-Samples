#ifndef CONFIG_H
#define CONFIG_H
#include <CubismFramework.hpp>
#include <iostream>
#include <QSettings>
class Config {
public:
    inline static const Csm::csmChar* ResourcesPath = "Resources/";
    inline static const Csm::csmChar* ModelDir = "Haru";
    inline static const Csm::csmInt32 ModelDirSize = sizeof(ModelDir) / sizeof(Csm::csmChar*);
    inline static const Csm::csmChar* MotionGroupIdle = "Idle";
    inline static const Csm::csmInt32 PriorityNone = 0;
    inline static const Csm::csmInt32 PriorityIdle = 1;
    inline static const Csm::csmInt32 PriorityNormal = 2;
    inline static const Csm::csmInt32 PriorityForce = 3;
    inline static const Csm::CubismFramework::Option::LogLevel CubismLoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
};
#endif
