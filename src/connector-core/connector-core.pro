
REPO_DIR = $$(TERBIT_CONNECTOR_HOME)

QT       += widgets qml

TARGET = connector-core
TEMPLATE = lib
CONFIG += staticlib

#be sure to include after settting REPO_DIR and TARGET
include($${REPO_DIR}/src/tools/qmaketerbit.pri)

DEFINES *= BUILD_ID_STR="\\\"$$(TERBIT_REV_ID)\\\""


SOURCES += \
    Workspace.cpp \
    LogDL.cpp \
    WorkspaceView.cpp \
    ../tools/Tools.cpp \
    ../tools/Log.cpp \
    CMItemTree.cpp \
    Plugin.cpp \
    ScriptFilesMRU.cpp \
    ResourceManager.cpp \
    DataClass.cpp \
    Actions.cpp \
    CoreFactory.cpp \
    AboutView.cpp \
    Block.cpp \
    WorkspaceDockWidget.cpp \
    ../tools/TerbitValue.cpp \
    ../tools/Script.cpp \
    LogView.cpp \
    OptionsDLView.cpp \
    OptionsDL.cpp \
    DataSource.cpp \
    DataClassManager.cpp \
    DataClassType.cpp \
    PluginsView.cpp \
    SystemView.cpp \
    DataSet.cpp \
    DataSetListView.cpp \
    ../tools/widgets/BigScrollbar.cpp \
    ScriptDocumentation.cpp \
    StartupShutdownApp.cpp \
    BlockIOContainer.cpp \
 
HEADERS += \
    Workspace.h \
    LogDL.h \
    WorkspaceView.h \
    ../tools/Tools.h \
    ../tools/Log.h \
    CMItemTree.h \
    Plugin.h \
    ScriptFilesMRU.h \
    ResourceManager.h \
    BuildInfoCore.h \
    DataClass.h \
    DataClassAction.h \
    Actions.h \
    IDataClassFactory.h \
    CoreFactory.h \
    AboutView.h \
    Block.h \
    WorkspaceDockWidget.h \
    ../tools/TerbitValue.h \
    ../tools/Script.h \
    LogView.h \
    OptionsDLView.h \
    OptionsDL.h \
    DataSource.h \
    DataClassManager.h \
    DataClassType.h \
    PluginsView.h \
    SystemView.h \
    DataSet.h \
    DataSetListView.h \
    ../tools/TerbitDefs.h \
    Event.h \
    ScriptDocumentation.h \
    StartupShutdownApp.h \
    BlockIOContainer.h \

#for lib, copy to standard spot
QMAKE_POST_LINK += $$TEMP_POST_LINK_LIB_COPY

RESOURCES += \
    resources/ConnectorCore.qrc

OTHER_FILES +=


