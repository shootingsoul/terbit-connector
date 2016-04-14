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
//! This class is used to create various QActions used throughout the app.
//! Ideally, an action would be created once, and used by multiple controls,
//! such as a menu item and a toolbar button.  Then the menu and button would
//! always be in synch because the action connected to each of them is enabled,
//! disabled, calls the same code, etc.
//! Unfortunately, things get more complicated when a set of actions are very
//! similar, but differ in one or more parameters.  For example, removing a
//! device is done the same way for each device, but the device ID differs
//! based on the device selected.  So a new action must be created for each
//! element.  To take advantage of what is shared, however, we created factory
//! functions in this file to create these commonly used actions.
#include "Workspace.h"
#include "DataClass.h"
#include <QAction>


namespace terbit
{
class DataClassAction;

class Actions : public QObject
{
   Q_OBJECT
public:
   Actions(Workspace *ws);
   //! creates an action that will remove the device indicated by devId
   DataClassAction* makeDevRemAction(DataClassAutoId_t devId, QObject* parent);
   //! creates an action that will show the control the device indicated by devId
   DataClassAction* makeDevShowAction(DataClassAutoId_t devId, QObject* parent);
   DataClassAction* makeSourceShowAction(DataClassAutoId_t devId, QObject* parent);
   DataClassAction* makeBufferShowAction(DataClassAutoId_t devId, QObject* parent);
   //! creates an action that will create a dialog box prompting for new device name
   DataClassAction* makeDevRenameAction(DataClassAutoId_t devId, QObject* parent);
   DataClassAction* makeDevCreateAction(DataClassAutoId_t pluginId, QObject* parent);
   DataClassAction* makeDisplayCreateAction(DataClassAutoId_t pluginId, const QIcon& icon, QObject* parent);
   DataClassAction* makeProcessorCreateAction(DataClassAutoId_t pluginId, const QIcon& icon, QObject* parent);
   DataClassAction* makeProcessorShowAction(DataClassAutoId_t devId, QObject* parent);

private slots:
   void onDeviceRemove(void);
   void onDeviceCtrlGui(void);
   void onBufferGui(void);
   void onSourceGui(void);
   void onDeviceRename(void);
   void OnInstanceCreate(void);
   void onProcessorOptionsGui();

private:
   Workspace* m_workspace;

};


}//terbit

