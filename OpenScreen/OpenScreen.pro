#-------------------------------------------------
#
# Project created by QtCreator 2018-12-02T18:38:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
contains(QT_ARCH, i386) {
    LIBPATH +=$$_PRO_FILE_PWD_/lib32
    DESTDIR  =$$_PRO_FILE_PWD_/bin32
} else {
    LIBPATH +=$$_PRO_FILE_PWD_/lib64
    DESTDIR  =$$_PRO_FILE_PWD_/bin64
}
CONFIG(debug,debug|release){
    message(Do Not Use Debug Mode Unless You Have Debug Libraries!)
}
LIBS +=libcef_dll_wrapper.lib libcef.lib user32.lib Ole32.lib ZipLib.lib lzma.lib bzip2.lib zlib.lib advapi32.lib wtsapi32.lib Shlwapi.lib

TARGET = OpenScreen
TEMPLATE = app
QMAKE_LFLAGS += /LTCG
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
    renderwindow.cpp \
    browserprocess.cpp \
    zipreader.cpp \
    ini.c \
    runner.cpp \
    settings.cpp

HEADERS += \
    renderwindow.h \
    browserprocess.h \
    zipreader.h \
    ini.h \
    settings.h \
    runner.h \
    compileheader.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
