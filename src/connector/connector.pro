REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

#for full qml support (quick)
#QT       += core gui widgets qml multimedia quick quickwidgets
QT       += core gui widgets qml multimedia webkitwidgets
TARGET = connector
TEMPLATE = app

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

TEMP_LIB_DIRS += $$TEMP_LIB_DIR_BUILD \
     $$TEMP_LIB_DIR_BOOST \
     $$TEMP_LIB_DIR_CRYPT_SSL \
     $$TEMP_LIB_DIR_IPHLPAPI

TEMP_LIBS = -lsignal-processing \
     -lmicrophone \
     -lfile-device \
     -lplots \
     -ldisplays \     
     -lconnector-core \
     -lscripting \
     $$TEMP_LIB_BOOST \
     $$TEMP_LIBS #order matters, put last

LIBS +=  $$TEMP_LIBS \
         $$TEMP_LIB_DIRS

HEADERS += ../connector-core/BuildInfoCore.h

SOURCES += \
    Main.cpp

OTHER_FILES +=

RC_FILE = connector.rc


