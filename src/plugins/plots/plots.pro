#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#include qml support
#QT       += widgets qml multimedia quick quickwidgets

QT       += widgets qml

TARGET   = plots
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += PLOTS_LIBRARY

SOURCES += \
    PlotsFactory.cpp \
    XYPlot.cpp \
    XYPlotPropertiesView.cpp \
    XYPlotRenderer.cpp \
    XYPlotView.cpp \
    XYSeries.cpp \
    XYSeriesRenderer.cpp \
    ../../tools/widgets/ScrollbarButton.cpp \
    ../../tools/widgets/ZoomScrollbar.cpp \
    ../../tools/widgets/ZoomScrollbarSlider.cpp

HEADERS += \
    PlotsFactory.h \
    Plots_global.h \
    XYPlot.h \
    XYPlotPropertiesView.h \
    XYPlotRenderer.h \
    XYPlotView.h \
    XYSeries.h \
    XYSeriesRenderer.h \
    ../../tools/widgets/ScrollbarButton.h \
    ../../tools/widgets/ZoomScrollbar.h \
    ../../tools/widgets/ZoomScrollbarSlider.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

OTHER_FILES +=

RESOURCES += \
    resources/plots.qrc


