#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#include qml support
#QT       += widgets qml multimedia quick quickwidgets

QT       += widgets qml multimedia

TARGET   = microphone
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += MICROPHONE_LIBRARY

SOURCES += \
    MicrophoneFactory.cpp \
    MicrophoneDevice.cpp \
    MicrophoneDeviceView.cpp

HEADERS += \
    MicrophoneDevice.h \
    Microphone_global.h \
    MicrophoneFactory.h \
    MicrophoneDeviceView.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

OTHER_FILES +=

RESOURCES += \
    resources/microphone.qrc


