QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# qDebug 打印是否输出
# DEFINES += QT_NO_DEBUG_OUTPUT

RC_ICONS = res/test.ico


SOURCES += \
    application/src/getimgapp.cpp \
    application/src/otapacketapp.cpp \
    view/src/getimgviewleft.cpp \
    view/src/getimgviewright.cpp \
    view/src/otapacketrightview.cpp \
    view/src/otapacketview.cpp \
    view/src/otapacketleftview.cpp \
    tool/src/logoutput.cpp \
    tool/src/configbase.cpp \
    tool/src/ringbuffer.cpp \
    tool/src/imgdecoder.cpp \
    view/src/controltabwidget.cpp \
    view/src/mainlogwidget.cpp \
    view/src/maintitlebar.cpp \
    view/src/getimgview.cpp \
    view/src/widget.cpp \
    main.cpp 

HEADERS += \
    application/inc/getimgapp.h \
    application/inc/otapacketapp.h \
    view/inc/getimgviewleft.h \
    view/inc/getimgviewright.h \
    view/inc/otapacketrightview.h \
    view/inc/otapacketview.h \
    view/inc/otapacketleftview.h \
    tool/inc/logoutput.h \
    tool/inc/alltoolfun.h \
    tool/inc/configbase.h \
    tool/inc/ringbuffer.h \
    tool/inc/imgdecoder.h \
    view/inc/controltabwidget.h \
    view/inc/mainlogwidget.h \
    view/inc/widget.h \
    view/inc/maintitlebar.h \
    view/inc/getimgview.h

FORMS += \
    view/ui/getimgviewleft.ui \
    view/ui/getimgviewright.ui \
    view/ui/getimgview.ui \
    view/ui/otapacketrightview.ui \
    view/ui/otapacketview.ui \
    view/ui/otapacketleftview.ui \
    view/ui/controltabwidget.ui \
    view/ui/mainlogwidget.ui \
    view/ui/maintitlebar.ui \
    view/ui/widget.ui

INCLUDEPATH += \
    view/inc \
    tool/inc \
    application/inc

RESOURCES += \
    res.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
