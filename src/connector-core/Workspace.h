/*
Copyright 2016 Codependable, LLC and Jonathan David Guerin

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <list>
#include <map>
#include <stdint.h>
#include <QEvent>
#include <QJSValue>
#include "DataSet.h"
#include "CoreFactory.h"
#include "Plugin.h"
#include "ScriptFilesMRU.h"
#include "Actions.h"
#include "DataClassManager.h"
#include <QDir>

namespace terbit
{

class WorkspaceView;
class WorkspaceDockWidget;
class DataSetListView;
class LogView;
class DataProviderAvailListView;
class DataProviderListView;
class SystemView;
class Actions;
class OptionsDL;
class ScriptProcessor;

class Workspace : public QObject
{
   Q_OBJECT

   friend class WorkspaceSW;
public:

   Workspace(OptionsDL* options);
   virtual ~Workspace();


   void Startup(QApplication& app);

   void ShowView();

   void ShutdownPrep();

   OptionsDL* GetOptions() { return m_options; }

   ScriptFilesMRU& GetScriptFilesMRU() { return m_scriptFilesMRU; }

   DataClassManager& GetDataClassManager() { return *m_dataClassManager; }


   DataSetListView* GetDataSetListView() { return m_dataSetListView; }

   LogView* GetLogView() { return m_logView; }

   //dynamic object creation for data classes
   DataSet* CreateDataSet(DataClass* owner, bool publicScope = true);
   DataClass* CreateInstance(DataClassAutoId_t typeId, DataClass* owner, bool publicScope = true);
   DataClass* CreateInstance(const QString& fullTypeName, DataClass* owner, bool publicScope = true);
   bool RenameInstance(DataClassAutoId_t id, const QString& str); //?? needed???
   DataClass* FindInstance(DataClassAutoId_t id);
   DataClass* FindInstance(const QString& uniqueId);
   DataClass *FindInstance(const QJSValue& value);


   PluginList& GetPlugins() { return m_plugins; }
   const DataClassTypeMap& GetDataClassTypes() { return m_dataClassManager->GetTypes(); }

   void RunScript(const QString& script, const QString& source, ScriptProcessor* sourceProcessor);
   void BuildDockAreaRestoreScript(ScriptBuilder& script, const QString& variableName);

   void OpenScriptFile(const QString& fileName);

   void WidgetAlert(QWidget *w);
   SystemView* GetDeviceListView(){return m_systemView;}
   Actions* GetActions(){return m_actions;}

   void AddDockWidget(WorkspaceDockWidget* w);   
   void RemDataClassDocks(DataClass* d);
   void FloatDockWidgetOutside(WorkspaceDockWidget* w);

   void DockRestoreGeometry(const QString& geometry);
   void DockRestoreState(const QString& state);

public slots:
      void DeleteInstance(DataClassAutoId_t id);

 signals:
   void ShowDataSetListView();
   void CloseDataSetListView();

   void ShowSystemView();
   void CloseSystemView();

   void ShowLogView();
   void CloseLogView();

   void InstanceCreated(DataClass* object);
   void DeleteInstanceThreaded(DataClassAutoId_t id);

private slots:
   void OnShowDataSetListView();
   void OnCloseDataSetListView();
   void OnDataSetListViewClosed();

   void OnShowSystemView();
   void OnCloseSystemView();
   void OnSystemViewClosed();

   void OnShowLogView();
   void OnCloseLogView();
   void OnLogViewClosed();

private:
   Workspace(const Workspace& o); //disable copy ctor

   void ApplyDarkTheme();
   void LoadPlugins(void);
   void LoadTranslations(QApplication& app);
   void LoadTranslationsDir(QApplication& app, QDir& dir, QLocale& locale);
   void RestoreSession(bool lastStartClean);
   void RestoreSessionAssignObjectNames();
   void SaveRestoreSessionState();

   DataClass* CreateInstance(DataClassType* type, DataClass* parent, bool publicScope);

   OptionsDL* m_options;

   WorkspaceView* m_view;

   DataSetListView* m_dataSetListView;
   SystemView* m_systemView;
   LogView* m_logView;

   ScriptFilesMRU m_scriptFilesMRU;

   CoreFactory m_coreFactory;
   DataClassManager* m_dataClassManager;
   PluginList m_plugins;
   Actions *m_actions;
   DataClassType* m_bufferType;

};

ScriptDocumentation* BuildScriptDocumentationWorkspace();

class WorkspaceSW : public QObject
{
   Q_OBJECT
public:
   WorkspaceSW(QJSEngine* se, Workspace* w, ScriptProcessor* sourceProcessor);

   //for use by restore script . . . . docking junks
   Q_INVOKABLE void AssignDockObjectNames();
   Q_INVOKABLE void DockRestoreGeometry(const QString& geometry);
   Q_INVOKABLE void DockRestoreState(const QString& state);
   Q_INVOKABLE QJSValue FindDock(const QString& objectName);

   Q_INVOKABLE void ShowDataSetsWindow();
   Q_INVOKABLE void ShowSystemViewWindow();
   Q_INVOKABLE void ShowLogViewerWindow();
   Q_INVOKABLE void CloseDataSetsWindow();
   Q_INVOKABLE void CloseSystemViewWindow();
   Q_INVOKABLE void CloseLogViewerWindow();

   Q_INVOKABLE QJSValue AddDataSet(const QJSValue &parent = QJSValue(), bool publicScope = true);
   Q_INVOKABLE QJSValue AddRemoteDataSet(const QJSValue& source, const QJSValue &owner = QJSValue(), bool publicScope = true);
   Q_INVOKABLE QJSValue Add(const QString& fullTypeName, const QJSValue& parent = QJSValue(), bool publicScope = true);
   Q_INVOKABLE void Remove(const QJSValue& instance);
   Q_INVOKABLE QJSValue Find(const QJSValue& uniqueId);

   Q_INVOKABLE void RunScript(const QString& sourceCode);
   Q_INVOKABLE void RunScriptFile(const QString& fileName);

protected:
   Workspace* m_workspace;
   QJSEngine* m_scriptEngine;
   ScriptProcessor* m_sourceProcessor;
};


}
