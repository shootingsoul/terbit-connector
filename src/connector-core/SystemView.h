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

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "tools/TerbitDefs.h"
#include "CMItemTree.h"
#include "WorkspaceDockWidget.h"
namespace terbit
{

class Workspace;
class DataClassType;
class Block;
class DataSet;
class DataSource;
class CMItemTree;

class SystemView : public WorkspaceDockWidget
{
   Q_OBJECT

public:
   SystemView(Workspace* workspace);

   QTreeWidget* TEST_GetTree() { return m_tree; }

private slots:
   void OnNameChanged(DataClass* d);
   void OnInstanceCreated(DataClass* dc);
   void RemoveObject(DataClass* dc);
   void OnOutputAdded(Block* block,DataClass* output);
   void OnDataSetIndexAssigned(DataSet* buf);

private:
   SystemView(const SystemView& o); //disable copy ctor

   void AddType(DataClassType* t);
   void AddObject(QTreeWidgetItem *pnode, DataClass* dc);
   QTreeWidgetItem* findNode(DataClassAutoId_t id);

   Workspace* m_workspace;
   CMItemTree* m_tree;
   QTreeWidgetItem* m_typeParent;
};

}
