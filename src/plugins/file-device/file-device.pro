#-------------------------------------------------
#
# Project created by QtCreator 2014-03-24T15:27:51
#
#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

QT       += widgets qml

TARGET   = file-device
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += FILEDVC_DP_LIBRARY

SOURCES += \
    ProgressLineEdit.cpp \
    FileDevice.cpp \
    FileDeviceView.cpp \
    FileDeviceViewWin.cpp \
    FileDeviceViewAdvanced.cpp \
    FileDeviceFactory.cpp \
    ../../tools/device/filedvc/filedvc.cpp

HEADERS += \
    ProgressLineEdit.h \
    ../../tools/device/filedvc/filedvc.h \
    FileDevice.h \
    FileDevice_global.h \
    FileDeviceView.h \
    FileDeviceViewWin.h \
    FileDeviceViewAdvanced.h \
    FileDeviceFactory.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

RESOURCES +=

TRANSLATIONS = \
    $${REPO_DIR}/datalight/translations/terbit_filedvc.tr.ts \
    $${REPO_DIR}/datalight/translations/terbit_filedvc.fr.ts \
    $${REPO_DIR}/datalight/translations/terbit_filedvc.sv.ts
