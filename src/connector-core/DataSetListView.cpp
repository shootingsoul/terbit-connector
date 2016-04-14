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

#include "DataSetListView.h"
#include "CMItemTree.h"
#include "Workspace.h"

namespace terbit
{

DataSetListView::DataSetListView(Workspace *workspace) : WorkspaceDockWidget(NULL, ""), m_workspace(workspace)
{
   m_tree = new CMItemTree(m_workspace);
   m_tree->setDragEnabled(true);
   m_tree->setHeaderHidden(true);


   for(auto& i : m_workspace->GetDataClassManager().GetInstances())
   {
      DataClass* dc = i.second;
      OnInstanceCreated(dc);
   }

   setWidget(m_tree);
   setWindowTitle(tr("Data Sets"));

   connect(m_workspace,SIGNAL(InstanceCreated(DataClass*)),this,SLOT(OnInstanceCreated(DataClass*)));

}

void DataSetListView::OnInstanceCreated(DataClass *dc)
{
   if (dc->GetPublicScope() && dc->IsDataSet())
   {
      //don't add index data sets twice, let them be added by their main data set . . .
      DataSet* buf = static_cast<DataSet*>(dc);
      if (buf->GetOwner() == NULL || !buf->GetOwner()->IsDataSet())
      {
         AddDataSet(buf);
      }
   }
}

void DataSetListView::AddDataSet(DataSet* buf)
{

   if (buf->GetPublicScope())
   {
      QTreeWidgetItem *node = NULL;
      QTreeWidgetItem *pnode = NULL;
      DataClass* parent = buf->GetOwner();
      if (parent && parent->IsDataSet())
      {
         pnode = findNode(parent->GetAutoId());
         if (pnode)
         {
            node = new QTreeWidgetItem(pnode, ItemDataSetInstance);
         }
      }
      else
      {
         node = new QTreeWidgetItem(m_tree, ItemDataSetInstance);
      }

      if (node)
      {
         m_tree->SetDataLoading(true);
         node->setText(0, buf->GetName());
         node->setData(0, Qt::UserRole, buf->GetAutoId());
         node->setIcon(0, QIcon(":/images/32x32/database_green.png"));
         node->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
         connect(buf, SIGNAL(NameChanged(DataClass*)), this, SLOT(ChangeDSName(DataClass*)));
         connect(buf,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(RemoveDataSet(DataClass*)));
         connect(buf,SIGNAL(IndexAssigned(DataSet*)),this, SLOT(OnDataSetIndexAssigned(DataSet*)));
         m_tree->SetDataLoading(false);

         if (buf->GetIndexDataSet() != NULL)
         {
            AddDataSet(buf->GetIndexDataSet());
         }

         if (pnode)
         {
            pnode->setExpanded(true);
         }
      }
   }
}


void DataSetListView::ChangeDSName(DataClass* buf)
{
   //iterate through nodes, find the right one, change text
   QTreeWidgetItemIterator it(m_tree);
   while (*it)
   {
      if ((*it)->data(0,Qt::UserRole).toUInt() == buf->GetAutoId())
      {
         m_tree->SetDataLoading(true);
         (*it)->setText(0, buf->GetName());
         m_tree->SetDataLoading(false);
         break;
      }
      ++it;
   }

   if(NULL == *it)
   {
      ; // TODO: log we didn't find data set
   }
}


void DataSetListView::RemoveDataSet(DataClass* dc)
{
   DataClassAutoId_t id = dc->GetAutoId();
   QTreeWidgetItemIterator it(m_tree);
   while (*it)
   {
      if ((*it)->data(0,Qt::UserRole).toUInt() == id)
      {
         delete (*it); //remove from tree
         break;
      }
      ++it;
   }
}

void DataSetListView::OnDataSetIndexAssigned(DataSet *buf)
{
   QTreeWidgetItem* node = findNode(buf->GetAutoId());

   if (node)
   {
      if (node->childCount() > 0)
      {
         delete node->child(0);
      }
      if (buf->GetIndexDataSet())
      {
         AddDataSet(buf->GetIndexDataSet());
      }
   }
}


QTreeWidgetItem* DataSetListView::findNode(uint32_t id)
{
   QTreeWidgetItemIterator it(m_tree);
   while (*it)
   {
      if(id == (*it)->data(0, Qt::UserRole).toUInt())
      {
         break;
      }
      ++it;
   }
   return (*it);
}


}
