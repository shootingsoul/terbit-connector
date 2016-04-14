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
#include "CMItemTree.h"
#include "DataSet.h"
#include "WorkspaceDockWidget.h"

namespace terbit
{

class Workspace;

class DataSetListView : public WorkspaceDockWidget
{
   Q_OBJECT

public:
   DataSetListView(Workspace* workspace);

   QTreeWidget* TEST_GetTree() { return m_tree; }


private slots:
   void OnInstanceCreated(DataClass* buf);
   void ChangeDSName(DataClass*);
   void RemoveDataSet(DataClass *dc);
   void OnDataSetIndexAssigned(DataSet*buf);

private:
   DataSetListView(const DataSetListView& o); //disable copy ctor

   void AddDataSet(DataSet* buf);

   QTreeWidgetItem* findNode(uint32_t id);

   Workspace* m_workspace;
   CMItemTree* m_tree;

};

}
