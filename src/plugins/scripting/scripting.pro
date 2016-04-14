#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#include qml support
#QT       += widgets qml multimedia quick quickwidgets

QT       += widgets qml webkitwidgets

TARGET   = scripting
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += SCRIPTING_LIBRARY

SOURCES += \
    ScriptingFactory.cpp \
    ScriptProcessor.cpp \
    ScriptProcessorView.cpp \
    ../../tools/widgets/CodeEditor.cpp \
    ScriptBuilder.cpp \
    FrequencyMetricsLibSW.cpp \
    ../../tools/FrequencySignalMetrics.cpp \
    ScriptingUtils.cpp \
    TerbitSW.cpp \
    FileIOLibSW.cpp \
    ScriptDisplay.cpp \
    ScriptDisplayView.cpp \
    ScriptDialogView.cpp \
    TimerLibSW.cpp

HEADERS += \
    ScriptProcessor.h \
    Scripting_global.h \
    ScriptingFactory.h \
    ScriptProcessorView.h \
    ../../tools/widgets/CodeEditor.h \
    ScriptBuilder.h \
    FrequencyMetricsLibSW.h \
    ../../tools/FrequencySignalMetrics.h \
    ScriptingUtils.h \
    TerbitSW.h \
    FileIOLibSW.h \
    ../../tools/Tools.h \
    ScriptDisplay.h \
    ScriptDisplayView.h \
    ScriptDialogView.h \
    TimerLibSW.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

RESOURCES += \ 
    resources/scripting.qrc


