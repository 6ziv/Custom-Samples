#pragma once
#include <QOpenGLFunctions>
#include <CubismFramework.hpp>
#include <Model/CubismUserModel.hpp>
#include <ICubismModelSetting.hpp>
#include <Type/csmRectF.hpp>
#include <Rendering/OpenGL/CubismOffscreenSurface_OpenGLES2.hpp>
#include "qdebug.h"
class Model : public Csm::CubismUserModel
{
public:
    Model();
    virtual ~Model();
    void LoadAssets(const Csm::csmChar* dir, const  Csm::csmChar* fileName);
    void Update();
    void Draw();
    Csm::CubismMotionQueueEntryHandle StartMotion(const Csm::csmChar* group, Csm::csmInt32 no, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL);
    Csm::CubismMotionQueueEntryHandle StartRandomMotion(const Csm::csmChar* group, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL);
    void SetExpression(const Csm::csmChar* expressionID);
    void SetRandomExpression();
    virtual Csm::csmBool HitTest(const Csm::csmChar* hitAreaName, Csm::csmFloat32 x, Csm::csmFloat32 y);
    virtual Csm::csmBool TestMouse(Csm::csmFloat32 x, Csm::csmFloat32 y);
    Csm::Rendering::CubismOffscreenFrame_OpenGLES2& GetRenderBuffer();
    inline void move(long double x,long double y){
        offsetx+=x;offsety+=y;
    }
    inline std::pair<long double,long double>getpos(){
        return std::make_pair(offsetx,offsety);
    }
private:
    long double offsetx=0.8;
    long double offsety=-0.5;
    void SetupModel(Csm::ICubismModelSetting* setting);
    void SetupTextures();
    void PreloadMotionGroup(const Csm::csmChar* group);
    void ReleaseMotionGroup(const Csm::csmChar* group) const;
    void ReleaseMotions();
    void ReleaseExpressions();

    Csm::ICubismModelSetting* _modelSetting;
    Csm::csmString _modelHomeDir;
    Csm::csmFloat32 _userTimeSeconds;
    Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds;
    Csm::csmVector<Csm::CubismIdHandle> _lipSyncIds;
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>   _motions;
    Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>   _expressions;
    Csm::csmVector<Csm::csmRectF> _hitArea;
    Csm::csmVector<Csm::csmRectF> _userArea;
    const Csm::CubismId* _idParamAngleX;
    const Csm::CubismId* _idParamAngleY;
    const Csm::CubismId* _idParamAngleZ;
    const Csm::CubismId* _idParamBodyAngleX;
    const Csm::CubismId* _idParamEyeBallX;
    const Csm::CubismId* _idParamEyeBallY;

    Csm::Rendering::CubismOffscreenFrame_OpenGLES2  _renderBuffer;   ///< フレームバッファ以外の描画先
};



