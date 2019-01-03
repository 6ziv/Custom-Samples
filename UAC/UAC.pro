#-------------------------------------------------
#
# Project created by QtCreator 2018-12-28T16:15:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = consent
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
LIBS+=user32.lib advapi32.lib crypt32.lib Wintrust.lib shlwapi.lib Gdi32.lib
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
contains(QT_ARCH,i386){
    QMAKE_LFLAGS+= /LARGEADDRESSAWARE /MACHINE:X86 /NOLOGO /RELEASE /INTEGRITYCHECK
}else{
    QMAKE_LFLAGS+= /LARGEADDRESSAWARE /MACHINE:X64 /NOLOGO /RELEASE /INTEGRITYCHECK
}
CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    certhandler.cpp

HEADERS += \
        mainwindow.h \
    certhandler.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc
