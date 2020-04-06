QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 release
INCLUDEPATH +=Framework/
INCLUDEPATH +=$$_PRO_FILE_PWD_/../../../3rdparty/include
LIBPATH +=$$_PRO_FILE_PWD_/../../../3rdparty/libs

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DesktopNotificationManagerCompat.cpp \
    Framework/CubismCdiJson.cpp \
    Framework/CubismDefaultParameterId.cpp \
    Framework/CubismFramework.cpp \
    Framework/CubismModelSettingJson.cpp \
    Framework/Effect/CubismBreath.cpp \
    Framework/Effect/CubismEyeBlink.cpp \
    Framework/Effect/CubismPose.cpp \
    Framework/Id/CubismId.cpp \
    Framework/Id/CubismIdManager.cpp \
    Framework/Math/CubismMath.cpp \
    Framework/Math/CubismMatrix44.cpp \
    Framework/Math/CubismModelMatrix.cpp \
    Framework/Math/CubismTargetPoint.cpp \
    Framework/Math/CubismVector2.cpp \
    Framework/Math/CubismViewMatrix.cpp \
    Framework/Model/CubismMoc.cpp \
    Framework/Model/CubismModel.cpp \
    Framework/Model/CubismModelUserData.cpp \
    Framework/Model/CubismModelUserDataJson.cpp \
    Framework/Model/CubismUserModel.cpp \
    Framework/Motion/ACubismMotion.cpp \
    Framework/Motion/CubismExpressionMotion.cpp \
    Framework/Motion/CubismMotion.cpp \
    Framework/Motion/CubismMotionJson.cpp \
    Framework/Motion/CubismMotionManager.cpp \
    Framework/Motion/CubismMotionQueueEntry.cpp \
    Framework/Motion/CubismMotionQueueManager.cpp \
    Framework/Physics/CubismPhysics.cpp \
    Framework/Physics/CubismPhysicsJson.cpp \
    Framework/Rendering/CubismRenderer.cpp \
    Framework/Rendering/OpenGL/CubismOffscreenSurface_OpenGLES2.cpp \
    Framework/Rendering/OpenGL/CubismRenderer_OpenGLES2.cpp \
    Framework/Type/csmRectF.cpp \
    Framework/Type/csmString.cpp \
    Framework/Utils/CubismDebug.cpp \
    Framework/Utils/CubismJson.cpp \
    Framework/Utils/CubismString.cpp \
    app.cpp \
    apputils.cpp \
    glwidget.cpp \
    main.cpp \
    menu.cpp \
    message.cpp \
    model.cpp \
    networkmgr.cpp \
    pluginloader.cpp \
    setup.cpp \
    textureloader.cpp

HEADERS += \
    DesktopNotificationManagerCompat.h \
    Framework/CubismCdiJson.hpp \
    Framework/CubismDefaultParameterId.hpp \
    Framework/CubismFramework.hpp \
    Framework/CubismFrameworkConfig.hpp \
    Framework/CubismModelSettingJson.hpp \
    Framework/Effect/CubismBreath.hpp \
    Framework/Effect/CubismEyeBlink.hpp \
    Framework/Effect/CubismPose.hpp \
    Framework/ICubismAllocator.hpp \
    Framework/ICubismModelSetting.hpp \
    Framework/Id/CubismId.hpp \
    Framework/Id/CubismIdManager.hpp \
    Framework/Live2DCubismCore.hpp \
    Framework/Math/CubismMath.hpp \
    Framework/Math/CubismMatrix44.hpp \
    Framework/Math/CubismModelMatrix.hpp \
    Framework/Math/CubismTargetPoint.hpp \
    Framework/Math/CubismVector2.hpp \
    Framework/Math/CubismViewMatrix.hpp \
    Framework/Model/CubismMoc.hpp \
    Framework/Model/CubismModel.hpp \
    Framework/Model/CubismModelUserData.hpp \
    Framework/Model/CubismModelUserDataJson.hpp \
    Framework/Model/CubismUserModel.hpp \
    Framework/Motion/ACubismMotion.hpp \
    Framework/Motion/CubismExpressionMotion.hpp \
    Framework/Motion/CubismMotion.hpp \
    Framework/Motion/CubismMotionInternal.hpp \
    Framework/Motion/CubismMotionJson.hpp \
    Framework/Motion/CubismMotionManager.hpp \
    Framework/Motion/CubismMotionQueueEntry.hpp \
    Framework/Motion/CubismMotionQueueManager.hpp \
    Framework/Physics/CubismPhysics.hpp \
    Framework/Physics/CubismPhysicsInternal.hpp \
    Framework/Physics/CubismPhysicsJson.hpp \
    Framework/Rendering/CubismRenderer.hpp \
    Framework/Rendering/OpenGL/CubismOffscreenSurface_OpenGLES2.hpp \
    Framework/Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp \
    Framework/Type/CubismBasicType.hpp \
    Framework/Type/csmMap.hpp \
    Framework/Type/csmRectF.hpp \
    Framework/Type/csmString.hpp \
    Framework/Type/csmVector.hpp \
    Framework/Utils/CubismDebug.hpp \
    Framework/Utils/CubismJson.hpp \
    Framework/Utils/CubismString.hpp \
    allocator.hpp \
    app.hpp \
    apputils.hpp \
    config.hpp \
    glwidget.hpp \
    hotkey.h \
    menu.hpp \
    message.h \
    model.hpp \
    musicplayer.hpp \
    networkmgr.h \
    openglhelper.hpp \
    pluginhelper.h \
    pluginloader.hpp \
    settings.h \
    setup.h \
    textureloader.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    icons.qrc
