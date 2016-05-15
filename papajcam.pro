TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    lightdetection.cpp \
    videorecorder.cpp

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gstreamermm-1.0

HEADERS += \
    lightdetection.h \
    videorecorder.h

unix: PKGCONFIG += sigc++-2.0
unix: PKGCONFIG += glibmm-2.4
unix: PKGCONFIG += giomm-2.4
