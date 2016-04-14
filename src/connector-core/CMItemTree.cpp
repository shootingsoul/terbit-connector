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

#include "CMItemTree.h"
#include <QMenu>
#include <QTreeWidgetItem>
#include <QPoint>
#include <QMessageBox>
#include "Workspace.h"
#include "DataClassAction.h"
#include "Actions.h"
#include "Block.h"
#include "LogDL.h"

namespace terbit
{



CMItemTree::CMItemTree(Workspace *pWs) : m_workspace(pWs), m_dataLoading(false)
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  //setSelectionMode(QAbstractItemView::MultiSelection);
  setExpandsOnDoubleClick(false);
  setAnimated(true);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  connect(this,
          SIGNAL(customContextMenuRequested(const QPoint&)),
          SLOT(onCustomContextMenuRequested(const QPoint&)));
  connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
          this, SLOT(onItemChanged(QTreeWidgetItem*, int)));
  connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
          this, SLOT(OnItemDoubleClicked(QTreeWidgetItem*,int)));

}

void CMItemTree::onCustomContextMenuRequested(const QPoint& pos)
{
   QTreeWidgetItem* item = itemAt(pos);

   if (item)
   {
      // Note: We must map the point to global from the viewport to
      // account for the header.
      showItemContextMenu(item, viewport()->mapToGlobal(pos));
   }
   else
   {
      showTreeCMenu(viewport()->mapToGlobal(pos));
   }
}


void CMItemTree::showItemContextMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
   QMenu menu;
   switch (item->type())
   {
       case ItemDeviceType:
           doDPAvailCMenu(item, globalPos);
           break;

       case ItemDeviceInstance:
           doDPActiveCMenu(item, globalPos);
           break;
      case ItemDataSetInstance:
          doDSCMenu(item, globalPos);
          break;
      case ItemProcessorInstance:
            doProcessorCMenu(item, globalPos);
            break;
      case ItemDataSourceInstance:
         doSourceCMenu(item, globalPos);
         break;
      default:
         menu.addAction(tr("No actions are defined for this item"));
         menu.exec();
         break;
   }
}

void CMItemTree::showTreeCMenu(const QPoint& globalPos)
{
   QMenu menu;

   // expand all
   DataClassAction* pAct = new DataClassAction(tr("Expand all"), this);
   pAct->setStatusTip(tr("Expand tree.  No effect on devices or data sets."));
   pAct->setIcon(QIcon(":/images/32x32/view-sort-ascending-2.png"));
   connect(pAct, SIGNAL(triggered()), this, SLOT(onExpandAll()));
   menu.addAction(pAct);

   pAct = new DataClassAction(tr("Collapse all"), this);
   pAct->setStatusTip(tr("Collapse tree.  No effect on devices or data sets."));
   pAct->setIcon(QIcon(":/images/32x32/view-sort-descending-2.png"));
   connect(pAct, SIGNAL(triggered()), this, SLOT(onCollapseAll()));
   menu.addAction(pAct);

   // collapse all
   menu.exec(globalPos);
}

void CMItemTree::doDPAvailCMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
   QMenu menu;

   DataClassAction* pAct = m_workspace->GetActions()->makeDevCreateAction(item->data(0, Qt::UserRole).toInt(), this);

   if(pAct)
   {
      menu.addAction(pAct);
   }

   menu.exec(globalPos);
}

void CMItemTree::doDPActiveCMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
   QMenu menu;
   DataClassAction* action;

   action = m_workspace->GetActions()->makeDevShowAction(item->data(0, Qt::UserRole).toInt(), this); // gets re-parented
   if(action)
   {
      menu.addAction(action);
   }

   // Cannot delegate this function to Actions because it uses itemTree edit functions
   action = new DataClassAction(tr("Rename..."), this);
   action->setStatusTip(tr("Rename this device instance."));
   action->m_DPId = item->data(0, Qt::UserRole).toInt();
   action->m_pTreeItem = item;
   action->setIcon(QIcon(":/images/32x32/vcard_edit.png"));
   connect(action, SIGNAL(triggered()), this, SLOT(onRename()));
   menu.addAction(action);

   action = m_workspace->GetActions()->makeDevRemAction(item->data(0, Qt::UserRole).toInt(), this); // gets re-parented
   if(action)
   {
      menu.addAction(action);
   }

   menu.exec(globalPos);
}

void CMItemTree::doProcessorCMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
   QMenu menu;

   DataClassAction* action;

   action = new DataClassAction(tr("Refresh"), this);
   action->m_DSId = item->data(0, Qt::UserRole).toInt();
   action->m_pTreeItem = item;
   action->setIcon(QIcon(":/images/32x32/function.png"));
   action->setToolTip(tr("Perform the calculation."));
   connect(action, SIGNAL(triggered()), this, SLOT(OnProcessorCalculate()));
   menu.addAction(action);

   action = m_workspace->GetActions()->makeProcessorShowAction(item->data(0, Qt::UserRole).toInt(), this); // gets re-parented
   if(action)
   {
      menu.addAction(action);
   }

   action = new DataClassAction(tr("Remove Processor"), this);
   action->m_DSId = item->data(0, Qt::UserRole).toInt();
   action->m_pTreeItem = item;
   action->setIcon(QIcon(":/images/32x32/delete.png"));
   action->setToolTip(tr("Remove the data processor."));
   connect(action, SIGNAL(triggered()), this, SLOT(OnRemove()));
   menu.addAction(action);

   menu.exec(globalPos);
}

void CMItemTree::doSourceCMenu(QTreeWidgetItem *item, const QPoint &globalPos)
{
   QMenu menu;
   DataClassAction* a;
   DataSource* source = static_cast<DataSource*>(m_workspace->FindInstance(item->data(0, Qt::UserRole).toInt()));
   if (!source)
   {
      return;
   }

   if (source->GetDisplayViewTypeName().length() > 0)
   {
      AddNewDefaultDisplayAction(&menu,source->GetDisplayViewTypeName(),source->GetAutoId(),true);
   }
   QMenu* pSubMenu = menu.addMenu(tr("Add to Display"));
   BuildAddDisplayMenu(pSubMenu,item->data(0, Qt::UserRole).toUInt(), true);
   pSubMenu->addSeparator();


   if (source && source->GetReadable())
   {      
      a = new DataClassAction(tr("Create Remote Data Set"), this);
      a->m_DSId = item->data(0, Qt::UserRole).toInt();
      a->m_pTreeItem = item;
      a->setToolTip(tr("Creates a new data set based on this data source"));
      connect(a, SIGNAL(triggered()), this, SLOT(OnSourceCreateRemote()));
      menu.addAction(a);
   }

   a = new DataClassAction(tr("Rename..."), this);
   a->setStatusTip(tr("Rename this item."));
   a->m_DSId = item->data(0, Qt::UserRole).toInt();
   a->setIcon(QIcon(":/images/32x32/vcard_edit.png"));
   a->m_pTreeItem = item;
   connect(a, SIGNAL(triggered()), this, SLOT(onRename()));
   menu.addAction(a);

   menu.exec(globalPos);
}
void CMItemTree::doDSCMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
   QMenu menu;
   DataClassAction* action;

   DataSet* ds = static_cast<DataSet*>(m_workspace->FindInstance(item->data(0, Qt::UserRole).toInt()));
   if (!ds)
   {
      return;
   }

   //Add to display . . . .
   if (ds->GetDisplayViewTypeName() > 0)
   {
      AddNewDefaultDisplayAction(&menu,ds->GetDisplayViewTypeName(),ds->GetAutoId(),false);
   }
   QMenu* pSubMenu = menu.addMenu(tr("Add to Display"));
   BuildAddDisplayMenu(pSubMenu, ds->GetAutoId(), false);
   pSubMenu->addSeparator();

   //Processors . . .
   pSubMenu = menu.addMenu(tr("Apply Function"));
   BuildAddProcessorMenu(pSubMenu, ds->GetAutoId());

   action = new DataClassAction(tr("Show Values"), this);
   action->m_DSId = item->data(0, Qt::UserRole).toInt();
   action->setIcon(QIcon(":images/32x32/table.png"));
   action->m_pTreeItem = item;
   connect(action, SIGNAL(triggered()), this, SLOT(OnDsValues()));
   menu.addAction(action);

   if (ds->CanRefreshData())
   {
      action = new DataClassAction(tr("Refresh Data"), this);
      action->m_DSId = item->data(0, Qt::UserRole).toInt();
      action->m_pTreeItem = item;
      action->setToolTip(tr("Update the data in the data set from the input source."));
      connect(action, SIGNAL(triggered()), this, SLOT(OnDsRefresh()));
      menu.addAction(action);
   }

   if (ds->GetReadable())
   {
      action = new DataClassAction(tr("Create Remote Data Set"), this);
      action->m_DSId = item->data(0, Qt::UserRole).toInt();
      action->setIcon(QIcon(":/images/32x32/database_green.png"));
      action->m_pTreeItem = item;
      action->setToolTip(tr("Creates a new data set based on this data set"));
      connect(action, SIGNAL(triggered()), this, SLOT(OnDsCreateRemote()));
      menu.addAction(action);
   }

   action = new DataClassAction(tr("Rename..."), this);
   action->setStatusTip(tr("Rename this item."));
   action->m_DSId = item->data(0, Qt::UserRole).toInt();
   action->setIcon(QIcon(":/images/32x32/vcard_edit.png"));
   action->m_pTreeItem = item;
   connect(action, SIGNAL(triggered()), this, SLOT(onRename()));
   menu.addAction(action);


   if (ds->GetOwner() == NULL)
   {
      action = new DataClassAction(tr("Remove Data Set"), this);
      action->m_DSId = item->data(0, Qt::UserRole).toInt();
      action->m_pTreeItem = item;
      action->setIcon(QIcon(":/images/32x32/delete.png"));
      action->setToolTip(tr("Remove the data set."));
      connect(action, SIGNAL(triggered()), this, SLOT(OnRemove()));
      menu.addAction(action);
   }

   menu.exec(globalPos);
}

void CMItemTree::BuildAddDisplayMenu(QMenu* menu, DataClassAutoId_t dcToApply, bool isSource)
{
   menu->setIcon(QIcon(":/images/32x32/linechart.png"));

   //list new displays
   QMenu* addNewMenu = menu->addMenu(QIcon(":/images/32x32/add.png"),tr("New Display"));
   for(auto& it : m_workspace->GetDataClassManager().GetTypes())
   {
      DataClassType* type = it.second;
      if (type->GetKind() == DATA_CLASS_KIND_DISPLAY)
      {
         AddNewDisplayAction(addNewMenu,type->GetDisplayName(), type->GetGraphics32(),type,dcToApply,isSource);
      }
   }

   //list existing displays we can add to
   menu->addSeparator();
   for(auto& it : m_workspace->GetDataClassManager().GetInstances())
   {
      DataClass* display = it.second;
      if (display->GetType()->GetKind() == DATA_CLASS_KIND_DISPLAY && display->GetPublicScope())
      {
         DataClassAction* action;
         action = new DataClassAction(display->GetName(), menu);
         action->m_DSId = dcToApply;
         action->m_DDId = display->GetAutoId();
         action->setIcon(display->GetType()->GetGraphics32());
         if (isSource)
         {
            connect(action, SIGNAL(triggered()), this, SLOT(onAddExistingDisplayForDataSource()));
         }
         else
         {
            connect(action, SIGNAL(triggered()), this, SLOT(onAddExistingDisplayForDataSet()));
         }
         menu->addAction(action);
      }
   }
}

void CMItemTree::AddNewDefaultDisplayAction(QMenu *pSubMenu, const QString& typeName, DataClassAutoId_t dcToApply, bool isSource)
{
   DataClassType* type = m_workspace->GetDataClassManager().FindType(typeName);
   if (type)
   {
      AddNewDisplayAction(pSubMenu,tr("Add to New %1").arg(type->GetDisplayName()),QIcon(":/images/32x32/add.png"), type, dcToApply, isSource);
   }
}

void CMItemTree::AddNewDisplayAction(QMenu *pSubMenu, const QString& name, const QIcon& icon, DataClassType* type, DataClassAutoId_t dcToApply, bool isSource)
{
   DataClassAction* action;
   action = new DataClassAction(name, pSubMenu);
   action->m_DSId = dcToApply;
   action->m_DDId = type->GetId();
   action->setIcon(icon);
   if (isSource)
   {
      connect(action, SIGNAL(triggered()), this, SLOT(onCreateDisplayForDataSource()));
   }
   else
   {
      connect(action, SIGNAL(triggered()), this, SLOT(onCreateDisplayForDataSet()));
   }
   pSubMenu->addAction(action);
}

#if 0
void CMItemTree::onDataProviderRemove(void)
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      m_workspace->RemoveDataProvider(act->m_DPId);
   }
}
#endif


void CMItemTree::onCreateDisplayForDataSet()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClassType* type = m_workspace->GetDataClassManager().FindType(act->m_DDId);
      if (type)
      {
         Block* b = static_cast<Block*>(m_workspace->CreateInstance(type->GetId(),NULL,true));
         DataClass* dc = m_workspace->FindInstance(act->m_DSId);
         if (b && dc)
         {
            b->InteractiveInit();
            b->ApplyInputDataClass(dc);
         }
         else
         {
           LogError(g_log.general,QObject::tr("Failed to create display for dataset"));
         }
      }
   }
}

void CMItemTree::onCreateDisplayForDataSource()
{

   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClassType* type = m_workspace->GetDataClassManager().FindType(act->m_DDId);
      DataSource* source = static_cast<DataSource*>(m_workspace->GetDataClassManager().Find(act->m_DSId));
      if (type && source)
      {
         Block* d = static_cast<Block*>(m_workspace->CreateInstance(type->GetId(),NULL,true));
         if (d)
         {
            d->InteractiveInit();
            DataSet* ds = CreateRemoteDataSet(source, NULL,true);
            if (ds)
            {
               ds->InteractiveInit();
               d->ApplyInputDataClass(ds);
            }
         }
      }
   }

}

void CMItemTree::onAddExistingDisplayForDataSet()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* block = m_workspace->FindInstance(act->m_DDId);
      DataClass* ds = m_workspace->FindInstance(act->m_DSId);
      if (block && ds && block->IsBlock())
      {
         static_cast<Block*>(block)->ApplyInputDataClass(ds);
      }
   }
}

void CMItemTree::onAddExistingDisplayForDataSource()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* block = m_workspace->FindInstance(act->m_DDId);
      DataSource* source = static_cast<DataSource*>(m_workspace->FindInstance(act->m_DSId));
      if (block && source && block->IsBlock())
      {
         DataSet* ds = CreateRemoteDataSet(source, NULL,true);
         if (ds)
         {
            ds->InteractiveInit();
            static_cast<Block*>(block)->ApplyInputDataClass(ds);
         }
      }
   }
}

void CMItemTree::BuildAddProcessorMenu(QMenu* menu, DataClassAutoId_t dcToApply)
{
   menu->setIcon(QIcon(":/images/32x32/function.png"));

   for(auto& it : m_workspace->GetDataClassManager().GetTypes())
   {
      DataClassType* type = it.second;
      if (type->GetKind() == DATA_CLASS_KIND_PROCESSOR)
      {
         AddNewProcessorAction(menu,type->GetDisplayName(), type->GetGraphics32(),type,dcToApply);
      }
   }
}


void CMItemTree::AddNewProcessorAction(QMenu *pSubMenu, const QString& name, const QIcon& icon, DataClassType* type, DataClassAutoId_t dcToApply)
{
   DataClassAction* action;
   action = new DataClassAction(name, pSubMenu);
   action->m_DSId = dcToApply;
   action->m_DDId = type->GetId();
   action->setIcon(icon);
   connect(action, SIGNAL(triggered()), this, SLOT(onCreateProcessorForDataClass()));
   pSubMenu->addAction(action);
}

void CMItemTree::onCreateProcessorForDataClass()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      DataClassType* type = m_workspace->GetDataClassManager().FindType(act->m_DDId);
      if (d && type)
      {
         Block* b = static_cast<Block*>(m_workspace->CreateInstance(type->GetId(),d,true));
         if (b)
         {
            b->InteractiveInit();
            b->ApplyInputDataClass(d);
         }
      }
   }
}

void CMItemTree::OnDsValues(void)
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      if (d && d->IsDataSet())
      {
         static_cast<DataSet*>(d)->ShowValuesView();
      }
   }
}

void CMItemTree::OnSourceCreateRemote()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      if (d && d->GetType()->GetKind() == DATA_CLASS_KIND_SOURCE)
      {
         DataSet* ds = CreateRemoteDataSet(static_cast<DataSource*>(d),NULL,true);
         ds->InteractiveInit();
      }
   }
}


void CMItemTree::OnDsCreateRemote()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      if (d && d->IsDataSet())
      {
         DataSet* ds = CreateRemoteDataSet(static_cast<DataSet*>(d), NULL,true);
         ds->InteractiveInit();
      }
   }
}

void CMItemTree::OnDsRefresh()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      if (d && d->IsDataSet())
      {
         static_cast<DataSet*>(d)->RefreshData();
      }
   }
}


void CMItemTree::OnRemove()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      //auto remote index data set too if it's parent is this dataset
      //TODO auto remove children???
      DataClass* dc = m_workspace->FindInstance(act->m_DSId);
      if (dc && dc->IsDataSet())
      {
         DataSet* ds = static_cast<DataSet*>(dc);
         if (ds->GetIndexDataSet() && ds->GetIndexDataSet()->GetOwner() == ds)
         {
            m_workspace->DeleteInstance(ds->GetIndexDataSet()->GetAutoId());
         }
      }
      m_workspace->DeleteInstance(act->m_DSId);
   }
}

void CMItemTree::OnProcessorCalculate()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->FindInstance(act->m_DSId);
      if (d && d->GetType()->GetKind() == DATA_CLASS_KIND_PROCESSOR)
      {
         static_cast<Block*>(d)->Refresh();
      }
   }

}


void CMItemTree::onRename()
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      editItem(act->m_pTreeItem, 0);
   }
}


void CMItemTree::onCollapseAll(void)
{
   collapseAll();
}

void CMItemTree::onExpandAll(void)
{
   expandAll();
}

void CMItemTree::onItemChanged(QTreeWidgetItem* pNode, int col)
{
   if(0 == col && !m_dataLoading)
   {
      if(pNode->type() == ItemDataSetInstance)
      {
         DataClassAutoId_t id = pNode->data(0, Qt::UserRole).toUInt();
         m_workspace->RenameInstance(id, pNode->text(0));
      }
      else if(pNode->type() == ItemDeviceInstance)
      {
         DataClassAutoId_t id = pNode->data(0, Qt::UserRole).toUInt();
         m_workspace->RenameInstance(id, pNode->text(0));
      }
      else if(pNode->type() == ItemDataSourceInstance)
      {
         DataClassAutoId_t id = pNode->data(0, Qt::UserRole).toUInt();
         m_workspace->RenameInstance(id, pNode->text(0));
      }
   }
}

void CMItemTree::OnItemDoubleClicked(QTreeWidgetItem * item, int column)
{
   DataClass* dv;
   DataClassAutoId_t id;
   column; // warning
   id = item->data(0,Qt::UserRole).toUInt();
   switch (item->type())
   {
   case ItemDeviceType:
      dv = m_workspace->CreateInstance(id, NULL);
      if (dv)
      {
         if(!dv->InteractiveInit())
         {
            //TODO: LogError(tr("Interactive Init for device %1 failed.").arg(id);
            m_workspace->DeleteInstance(dv->GetAutoId());
         }
         else
         {
            item->setExpanded(true);
         }
      }
      else
      {
         //TODO report
         ;// TODO: LogError(tr("Failed to create display object %1.").arg(id);
      }
      break;
   case ItemDeviceInstance:
   case ItemDataSetInstance:
   case ItemProcessorInstance:
   case ItemDataSourceInstance:
      {
         DataClass* d = m_workspace->FindInstance(id);
         if (d)
         {
            d->InteractiveFocus();
         }
      }
      break;
   default:
      break;
   }
}


}// end terbit

