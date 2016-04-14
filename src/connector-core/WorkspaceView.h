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

#include <QMainWindow>
#include <QAction>
#include "DataSet.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QStatusBar;
QT_END_NAMESPACE


namespace terbit
{

class WorkspaceDockWidget;
class Workspace;

class WorkspaceView : public QMainWindow
{
   Q_OBJECT

public:

   WorkspaceView(Workspace* workspace, QWidget *parent = 0);
   virtual ~WorkspaceView();

   void closeEvent(QCloseEvent * event);


public Q_SLOTS:
   void OnQuit();
   void OnAbout();
   void OnBufferListWindow();
   void OnSystemViewWindow();
   void OnLogWindow();
   void OnOpenScriptFileSelect();
   void OnOpenScriptFile();
   void OnClearScriptFileMRU();
   void OnOptions();
   void OnDeviceMenu();   
   void OnFileNewMenu();
   void OnWindowMenu();
   void OnMenuOpenWindowSelected();
   void OnPlugins();
private:
   void CreateMenus();   
   void RebuildRecentScriptsMenu();
   void OpenScriptFile(const QString& fileName);
   void makeDevMenu(QMenu* parent);
   QMenu* MakeDeviceTypeMenu(QMenu* parent);
   QMenu* MakeDisplayTypeMenu(QMenu* parent);
   QMenu* MakeProcessorTypeMenu(QMenu* parent);

   QAction* m_runScript;
   QMenu *m_menuRecentScripts;
   QMenu *m_menuDevices;
   QMenu *m_menuFileNew;
   QMenu *m_menuWindows;

   Workspace* m_workspace;

   //menu helper classes
   class WindowAction : public QAction
   {
   public:
      WindowAction(WorkspaceDockWidget* w, QObject* parent) : QAction(parent), dockWidget(w) {}
      WorkspaceDockWidget* dockWidget;
   };
};

}
