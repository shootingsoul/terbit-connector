#-------------------------------------------------

REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#include qml support
#QT       += widgets qml multimedia quick quickwidgets

QT       += widgets qml multimedia

TARGET   = signal-processing
TEMPLATE = lib
CONFIG   += plugin static

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES += SIGNALPROCESSING_LIBRARY

SOURCES += \
    SignalProcessingFactory.cpp \
    FFTProcessorView.cpp \
    ../../tools/DisplayFFT.cpp \
    ../../tools/kiss_fft.c \
    ../../tools/kiss_fftr.c \
    ../../tools/FrequencySignalMetrics.cpp \
    FrequencyMetricsDisplayView.cpp \
    ../../tools/SignalTools.cpp \
    ../../tools/widgets/ExtensionWidget.cpp \
    SignalAnalysisView.cpp \
    SigAnalysisProcessor.cpp \
    MetricsValueView.cpp \
    HarmonicsView.cpp \
    SigAnalysisProcSW.cpp

HEADERS += \
    SignalProcessing_global.h \
    SignalProcessingFactory.h \
    FFTProcessorView.h \
    ../../tools/DisplayFFT.h \
    ../../tools/kiss_fft.h \
    ../../tools/kiss_fftr.h \
    ../../tools/FrequencySignalMetrics.h \
    FrequencyMetricsDisplayView.h \
    ../../tools/SignalTools.h \
    ../../tools/widgets/ExtensionWidget.h \
    SignalAnalysisView.h \
    SigAnalysisProcessor.h \
    MetricsValueView.h \
    HarmonicsView.h \
    SigAnalysisProcSW.h

#QMAKE_CXXFLAGS += /showIncludes

#for lib, copy to standard spot
QMAKE_POST_LINK = $$TEMP_POST_LINK_LIB_COPY

OTHER_FILES +=

RESOURCES += \
    resources/signalprocessing.qrc
