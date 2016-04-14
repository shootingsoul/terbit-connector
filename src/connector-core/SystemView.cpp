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

#include "SystemView.h"
#include <QTreeWidgetItem>
#include "Workspace.h"
#include "Block.h"
#include "DataClass.h"
#include "DataSet.h"

namespace terbit
{

SystemView::SystemView(Workspace *workspace) : WorkspaceDockWidget(NULL, tr("System View")), m_workspace(workspace)
{
   m_tree = new(std::nothrow) CMItemTree(workspace);
   if(m_tree)
   {
      m_typeParent = new QTreeWidgetItem(m_tree);
      m_typeParent->setText(0,tr("Available Types"));
      m_typeParent->setIcon(0, QIcon(":/images/32x32/plugin_go.png"));
      m_typeParent->setExpanded(true);
      m_tree->setDragEnabled(true);
      m_tree->setHeaderHidden(true);
   }

   for(DataClassTypeMap::const_iterator i = m_workspace->GetDataClassTypes().begin(); i != m_workspace->GetDataClassTypes().end(); i++)
   {
      if ((i->second)->GetKind() == DATA_CLASS_KIND_DEVICE)
      {
         AddType(i->second);
      }
   }

   for(auto& it : m_workspace->GetDataClassManager().GetInstances())
   {
      //use same logic when new items are created in order to rebuild . . .
      OnInstanceCreated(it.second);
   }

   connect(m_workspace,SIGNAL(InstanceCreated(DataClass*)),this,SLOT(OnInstanceCreated(DataClass*)));

   setWidget(m_tree);
}


void SystemView::OnInstanceCreated(DataClass *dc)
{
   QTreeWidgetItem *pnode = NULL;

   switch (dc->GetType()->GetKind())
   {
   case DATA_CLASS_KIND_SOURCE:
      if (dc->GetOwner() == NULL)
      {
         AddObject(pnode, dc);
      }
      break;
   case DATA_CLASS_KIND_DEVICE:
      AddObject(pnode, dc);
      break;
   case DATA_CLASS_KIND_PROCESSOR:
      if (dc->GetOwner())
      {
         pnode = findNode(dc->GetOwner()->GetAutoId());
      }
      AddObject(pnode, dc);
      break;
   }
}

void SystemView::OnOutputAdded(Block *block, DataClass* output)
{
   QTreeWidgetItem *pnode = NULL;
   pnode = findNode(block->GetAutoId());
   if (pnode)
   {
      AddObject(pnode, output);
   }
}


void SystemView::OnDataSetIndexAssigned(DataSet *buf)
{
   QTreeWidgetItem *pnode = NULL;
   pnode = findNode(buf->GetAutoId());
   if (pnode)
   {
      if (pnode->childCount() > 0)
      {
         delete pnode->child(0);
      }
      if (buf->GetIndexDataSet() != NULL)
      {
         AddObject(pnode, buf->GetIndexDataSet());
      }
   }
}

void SystemView::AddType(DataClassType* t)
{
   m_tree->SetDataLoading(true);
   QTreeWidgetItem *node = new QTreeWidgetItem(m_typeParent, ItemDeviceType);
   node->setText(0, t->GetFullTypeName());
   node->setData(0, Qt::UserRole, t->GetId());
   node->setToolTip(0, tr("Device Plugin"));
   node->setIcon(0, QIcon(":/images/32x32/plugin_go.png"));
   m_tree->SetDataLoading(false);
}



void SystemView::AddObject(QTreeWidgetItem *pnode, DataClass* dc)
{
   if (dc->GetPublicScope())
   {
      TerbitTreeItemType_t nodeType;

      switch (dc->GetType()->GetKind())
      {
      case DATA_CLASS_KIND_DEVICE:
         nodeType = ItemDeviceInstance;
         break;
      case DATA_CLASS_KIND_PROCESSOR:
         nodeType = ItemProcessorInstance;
         break;
      case DATA_CLASS_KIND_SOURCE:
         if (dc->IsDataSet())
         {
            nodeType = ItemDataSetInstance;
         }
         else
         {
            nodeType = ItemDataSourceInstance;
         }
         break;
      }

      QTreeWidgetItem *node = NULL;
      if (pnode != NULL)
      {
         node = new QTreeWidgetItem(pnode, nodeType);
      }
      else
      {
         node = new QTreeWidgetItem(m_tree, nodeType);
      }
      if (node)
      {
         m_tree->SetDataLoading(true);
         node->setText(0, dc->GetName());
         node->setData(0, Qt::UserRole, dc->GetAutoId());
         node->setIcon(0, dc->GetType()->GetGraphics32());
         if (dc->IsDataSet())
         {
            node->setToolTip(0, tr("Data set provided by the above instance"));
         }
         else if (dc->GetType()->GetKind() == DATA_CLASS_KIND_DEVICE)
         {
            node->setToolTip(0, tr("Device Instance"));
         }
         node->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
         connect(dc, SIGNAL(NameChanged(DataClass*)), this, SLOT(OnNameChanged(DataClass*)));
         connect(dc, SIGNAL(BeforeDeletion(DataClass*)), this, SLOT(RemoveObject(DataClass*)));
         m_tree->SetDataLoading(false);

         //add source node for devices and processors
         if (dc->IsBlock())
         {
            //add any existing sources/ buffers
            Block* block = static_cast<Block*>(dc);

            connect(block,SIGNAL(OutputAdded(Block*,DataClass*)),this, SLOT(OnOutputAdded(Block*,DataClass*)));

            for(auto& source : block->GetOutputs())
            {
               AddObject(node, source.second);
            }
         }
         else if (dc->IsDataSet())
         {
            DataSet* ds = static_cast<DataSet*>(dc);

            connect(ds,SIGNAL(IndexAssigned(DataSet*)),this,SLOT(OnDataSetIndexAssigned(DataSet*)));
            if (ds->GetIndexDataSet() != NULL)
            {
               AddObject(node, ds->GetIndexDataSet());
            }
         }

         if (pnode)
         {
            pnode->setExpanded(true);
         }
      }
   }
}


void SystemView::RemoveObject(DataClass* dc)
{
   QTreeWidgetItem* node = findNode(dc->GetAutoId());
   if (node)
   {
      delete node;
   }
}

void SystemView::OnNameChanged(DataClass* d)
{
   //iterate through nodes, find the right one, change text
   QTreeWidgetItem* node = findNode(d->GetAutoId());

   if(NULL != node)
   {
      m_tree->SetDataLoading(true);
      node->setText(0, d->GetName());
      m_tree->SetDataLoading(false);
   }
}


QTreeWidgetItem* SystemView::findNode(uint32_t id)
{
   QTreeWidgetItemIterator it(m_tree);
   while (*it)
   {
      QTreeWidgetItem* node = *it;
      if(id == node->data(0, Qt::UserRole).toUInt())
      {
         break;
      }
      ++it;
   }
   return (*it);
}


}// end terbit
