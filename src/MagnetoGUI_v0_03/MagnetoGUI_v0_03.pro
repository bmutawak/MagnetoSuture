#-------------------------------------------------
#
# Project created by QtCreator 2019-01-22T13:28:02
#
#-------------------------------------------------

QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MagnetoGUI_v0_03
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

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    glwidget.cpp \
    opencvworker.cpp \
    dialog_settingswindow.cpp \
    Physics.cpp \
    roboclaw.cpp \
    serial.cc \
    win.cc \
    ImageSegmentation.cpp \
    ControlModule.cpp \
    pathpointmarker.cpp \
    graphicsviewer.cpp

HEADERS += \
    macros.h \
        mainwindow.h \
    point.h \
    glwidget.h \
    opencvworker.h \
    dialog_settingswindow.h \
    Physics.h \
    roboclaw.h \
    serial.h \
    v8stdint.h \
    win.h \
    ImageSegmentation.h \
    ControlModule.h \
    pathpointmarker.h \
    graphicsviewer.h

FORMS += \
        mainwindow.ui \
    dialog_settingswindow.ui

# for OpenGL
LIBS += -lopengl32
LIBS += opengl32.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

# for Eigen
INCLUDEPATH += $$PWD/../lib/Eigen
DEPENDPATH += $$PWD/../lib/Eigen

# for OpenCV 3.45
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/opencv/build/x64/vc15/lib/ -lopencv_world345
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/opencv/build/x64/vc15/lib/ -lopencv_world345d
else:unix: LIBS += -L$$PWD/../lib/opencv/build/x64/vc15/lib/ -lopencv_world345

INCLUDEPATH += $$PWD/../lib/opencv/build/include
DEPENDPATH += $$PWD/../lib/opencv/build/include

# for ArUco
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/opencv_contrib/x64/vc15/lib/ -lopencv_aruco345
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/opencv_contrib/x64/vc15/lib/ -lopencv_aruco345d
else:unix: LIBS += -L$$PWD/../lib/opencv_contrib/x64/vc15/lib/ -lopencv_aruco345

INCLUDEPATH += $$PWD/../lib/opencv_contrib/include
DEPENDPATH += $$PWD/../lib/opencv_contrib/include

