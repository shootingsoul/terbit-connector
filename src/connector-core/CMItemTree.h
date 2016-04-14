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
#include "tools/Tools.h"

#include <QAction>
#include <QPoint>

namespace terbit
{
class Workspace;
class DataClassType;

typedef enum
{
   // Qt documentation states that user types should begin at this value.
   ItemDeviceType  = QTreeWidgetItem::UserType,
   ItemDeviceInstance,
   ItemDataSetInstance,
   ItemProcessorInstance,
   ItemDataSourceInstance
}TerbitTreeItemType_t;

class CMItemTree : public QTreeWidget
{
   Q_OBJECT

public:
   CMItemTree(Workspace* pWs);
   
   void SetDataLoading(bool value) { m_dataLoading = value; }
   bool GetDataLoading() { return m_dataLoading; }

signals:
   
private slots:
   void onCustomContextMenuRequested(const QPoint& pos);
   void showItemContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void onCreateDisplayForDataSet();
   void onCreateDisplayForDataSource();
   void onCreateProcessorForDataClass();
   void onAddExistingDisplayForDataSet();
   void onAddExistingDisplayForDataSource();
   void OnDsValues();
   void OnSourceCreateRemote();
   void OnDsCreateRemote();
   void OnDsRefresh();
   void OnRemove();
   void OnProcessorCalculate();
   void onCollapseAll(void);
   void onExpandAll(void);
   void onRename(void);
   void onItemChanged(QTreeWidgetItem* pNode, int col);
   void OnItemDoubleClicked(QTreeWidgetItem * item, int column);


private:
   void showTreeCMenu(const QPoint& globalPos);
   void doDPAvailCMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void doDPActiveCMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void doDSCMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void doProcessorCMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void doSourceCMenu(QTreeWidgetItem* item, const QPoint& globalPos);
   void BuildAddDisplayMenu(QMenu *pSubMenu, DataClassAutoId_t dcToApply, bool isSource);
   void AddNewDefaultDisplayAction(QMenu *pSubMenu, const QString &typeName, DataClassAutoId_t dcToApply, bool isSource);
   void AddNewDisplayAction(QMenu *pSubMenu, const QString& name, const QIcon& icon, DataClassType* type, DataClassAutoId_t dcToApply, bool isSource);
   void BuildAddProcessorMenu(QMenu* menu, DataClassAutoId_t dcToApply);
   void AddNewProcessorAction(QMenu *pSubMenu, const QString& name, const QIcon& icon, DataClassType* type, DataClassAutoId_t dcToApply);

   Workspace* m_workspace;
   bool m_dataLoading;

};

}// end namespace terbit

