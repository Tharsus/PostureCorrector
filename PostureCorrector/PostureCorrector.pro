#-------------------------------------------------
#
# Project created by QtCreator 2018-02-15T14:07:37
#
#-------------------------------------------------

QT       += core gui \
            multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PostureCorrector
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += C:\dlib_mingw\

LIBS += -L"C:\dlib_mingw\build"
LIBS += -ldlib


INCLUDEPATH += C:\opencv\build\include

LIBS += C:\opencv_mingw_build\bin\libopencv_core340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_highgui340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_imgcodecs340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_imgproc340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_features2d340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_calib3d340.dll
LIBS += C:\opencv_mingw_build\bin\libopencv_videoio340.dll

RESOURCES += \
    res.qrc
