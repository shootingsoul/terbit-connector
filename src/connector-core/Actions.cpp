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

#include "Actions.h"
#include "DataClassAction.h"
#include <QObject>
#include <QInputDialog>
#include "LogDL.h"

namespace terbit
{


Actions::Actions(Workspace* ws) : m_workspace(ws)
{
   // Create global actions for about box, open script, etc.

}


// ------------ QAction factory functions ------------
DataClassAction *Actions::makeDevShowAction(DataClassAutoId_t devId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Show View"), parent);
   if(pAct)
   {
      pAct->setStatusTip(tr("Make the control GUI for this instance visible."));
      pAct->m_DPId = devId;
      pAct->setIcon(QIcon(":/images/32x32/find.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onDeviceCtrlGui()));
   }

   return pAct;
}

DataClassAction *Actions::makeSourceShowAction(DataClassAutoId_t devId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Show View"), parent);
   if(pAct)
   {
      pAct->setStatusTip(tr("Make the control GUI for this instance visible."));
      pAct->m_DSId = devId;
      pAct->setIcon(QIcon(":/images/32x32/find.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onSourceGui()));
   }

   return pAct;

}

DataClassAction *Actions::makeBufferShowAction(DataClassAutoId_t devId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Show View"), parent);
   if(pAct)
   {
      pAct->setStatusTip(tr("Make the control GUI for this instance visible."));
      pAct->m_DSId = devId;
      pAct->setIcon(QIcon(":/images/32x32/find.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onBufferGui()));
   }

   return pAct;

}

DataClassAction *Actions::makeProcessorShowAction(DataClassAutoId_t devId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Show Options"), parent);
   if(pAct)
   {
      pAct->setStatusTip(tr("Make the options GUI for this processor visible."));
      pAct->m_PIId = devId;
      pAct->setIcon(QIcon(":/images/32x32/find.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onProcessorOptionsGui()));
   }

   return pAct;
}


DataClassAction *Actions::makeDevRenameAction(DataClassAutoId_t devId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Rename..."), parent);
   if(pAct)
   {
      pAct->setStatusTip(tr("Rename this device instance."));
      pAct->m_DPId = devId;
      pAct->setIcon(QIcon(":/images/32x32/vcard_edit.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onDeviceRename()));
   }

   return pAct;
}

DataClassAction* Actions::makeDevRemAction(DataClassAutoId_t devId, QObject* parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(QObject::tr("Remove device"), parent);
   if(pAct)
   {
      pAct->setStatusTip(QObject::tr("Deactivate and remove this device instance."));
      pAct->m_DPId = devId;
      pAct->setIcon(QIcon(":/images/32x32/delete.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(onDeviceRemove()));
   }

   return pAct;
}
DataClassAction* Actions::makeDevCreateAction(DataClassAutoId_t typeId, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(tr("Open new device"), this);
   parent;
   if(pAct)
   {
      pAct->setStatusTip(tr("Create an instance of this type of device."));
      //pAct->setData(item->data(0, Qt::UserRole));
      pAct->m_PIId = typeId;
      pAct->setIcon(QIcon(":/images/32x32/plugin_go.png"));
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnInstanceCreate()));
   }

   return pAct;

}
DataClassAction* Actions::makeDisplayCreateAction(DataClassAutoId_t typeId, const QIcon& icon, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(tr("Open new display"), this);
   parent;
   if(pAct)
   {
      pAct->setStatusTip(tr("Create an instance of this type of display."));
      //pAct->setData(item->data(0, Qt::UserRole));
      pAct->m_PIId = typeId;
      pAct->setIcon(icon);
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnInstanceCreate()));
   }

   return pAct;

}

DataClassAction* Actions::makeProcessorCreateAction(DataClassAutoId_t typeId, const QIcon& icon, QObject *parent)
{
   DataClassAction* pAct = new(std::nothrow) DataClassAction(tr("Open new display"), this);
   parent;
   if(pAct)
   {
      pAct->setStatusTip(tr("Create an instance of this type of display."));
      //pAct->setData(item->data(0, Qt::UserRole));
      pAct->m_PIId = typeId;
      pAct->setIcon(icon);
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnInstanceCreate()));
   }

   return pAct;
}


// -------- SLOTS --------------
void Actions::onDeviceRemove(void)
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      m_workspace->DeleteInstance(act->m_DPId);
   }
}

void Actions::onDeviceCtrlGui(void)
{
    DataClassAction* act = static_cast<DataClassAction*>(sender());
    if(act)
    {
       DataClass *dv = m_workspace->FindInstance(act->m_DPId);
       if(NULL != dv)
       {
          if(!dv->InteractiveFocus())
          {
             // TODO: LogWarning(tr("Interactive Focus for device %1 failed.").arg(dv->GetId())
          }
       }
    }
}

void Actions::onBufferGui()
{
   //TODO?? what gui to show???
//   DataClassAction* act = static_cast<DataClassAction*>(sender());
//   if(act)
//   {
//      Device *dv = static_cast<Device*>(m_workspace->FindObject(act->m_DSId));
//      //TODO
//      if(NULL != dv)
//      {
//         if(!dv->InteractiveFocus())
//         {
//            // TODO: LogWarning(tr("Interactive Focus for device %1 failed.").arg(dv->GetId())
//         }
//      }
//   }
}

void Actions::onSourceGui()
{
   //TODO??? what gui to show???
//   DataClassAction* act = static_cast<DataClassAction*>(sender());
//   if(act)
//   {
//      DataSource *source = static_cast<DataSource*>(m_workspace->FindObject(act->m_DSId));
//      if(NULL != source)
//      {
//         if(!source->InteractiveFocus())
//         {
//            // TODO: LogWarning(tr("Interactive Focus for device %1 failed.").arg(dv->GetId())
//         }
//      }
//   }
}

void Actions::onProcessorOptionsGui(void)
{
    DataClassAction* act = static_cast<DataClassAction*>(sender());
    if(act)
    {
       DataClass *dc = m_workspace->FindInstance(act->m_PIId);
       if(NULL != dc)
       {
          if(!dc->InteractiveFocus())
          {
             // TODO: LogWarning(tr("Interactive Focus for device %1 failed.").arg(dv->GetId())
          }
       }
    }
}

void Actions::onDeviceRename(void)
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* dv = m_workspace->FindInstance(act->m_DPId);
      if(dv)
      {
         bool ok;
         QString text = QInputDialog::getText(NULL, tr("Rename device"),
                                              tr("New device name:"), QLineEdit::Normal,
                                              dv->GetName(), &ok);
         if (ok && !text.isEmpty())
         {
            m_workspace->RenameInstance(act->m_DPId, text);
         }
      }
      else
      {
         ;// TODO: log/report error
      }
   }
   else
   {
      ;//log weird error - invalid QAction in onDeviceRename
   }
}

void Actions::OnInstanceCreate(void)
{
   DataClassAction* act = static_cast<DataClassAction*>(sender());
   if(act)
   {
      DataClass* d = m_workspace->CreateInstance(act->m_PIId, NULL);
      if (d)
      {
         if(!d->InteractiveInit())
         {
            LogError(g_log.general,tr("Interactive Init on %1 failed.").arg(d->GetType()->GetFullTypeName()));
            m_workspace->DeleteInstance(d->GetAutoId());
         }
      }
      else
      {
         LogError(g_log.general,tr("Failed to create object from type %1.").arg(act->m_PIId));
      }
   }
}


}//terbit
