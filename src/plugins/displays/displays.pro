#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#include qml support
#QT       += widgets qml multimedia quick quickwidgets

QT       += widgets qml

TARGET   = displays
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += DISPLAYS_LIBRARY

SOURCES += \
    DisplaysFactory.cpp \
    ../../tools/widgets/BigScrollbar.cpp \
    ../../tools/widgets/BigScrollbarSlider.cpp \
    ../../tools/widgets/ScrollbarButton.cpp \
    DataSetValues.cpp \
    DataSetValuesView.cpp

HEADERS += \
    DisplaysFactory.h \
    Displays_global.h \
    ../../tools/widgets/BigScrollbar.h \
    ../../tools/widgets/BigScrollbarSlider.h \
    ../../tools/widgets/ScrollbarButton.h \
    DataSetValues.h \
    DataSetValuesView.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

OTHER_FILES +=

RESOURCES += \
    resources/displays.qrc


