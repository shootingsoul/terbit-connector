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

#include "WorkspaceView.h"
#include "Workspace.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QStatusBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include "LogDL.h"
#include "AboutView.h"
#include "OptionsDLView.h"
#include "BuildInfoCore.h"
#include "DataClassAction.h"
#include "WorkspaceDockWidget.h"
#include "PluginsView.h"
#include "DataClassType.h"

namespace terbit
{



WorkspaceView::WorkspaceView(Workspace* workspace, QWidget *parent) : QMainWindow(parent), m_workspace(workspace)
{
   this->setAttribute(Qt::WA_DeleteOnClose);
   this->setDockNestingEnabled(true);
   setObjectName("WorkspaceView");
   CreateMenus();
   statusBar()->hide();
   setStyleSheet(QString("QMainWindow::separator { background-color: %1; width: 4; height: 4px; }").arg(palette().color(QPalette::Highlight).name()));

   setWindowTitle(_STR_PRODUCT_NAME);
}

WorkspaceView::~WorkspaceView()
{
}

void WorkspaceView::closeEvent(QCloseEvent * event)
{
   event; // warning
   m_workspace->ShutdownPrep();
}

void WorkspaceView::OnQuit()
{
   //closing main workspace view will shutdown appplication cleanly
   this->close();
}

void WorkspaceView::OnAbout()
{
   AboutView a(this);
   a.exec();
}

void WorkspaceView::OnPlugins()
{
   PluginsView p(this,m_workspace->GetPlugins(),m_workspace->GetDataClassManager());
   p.exec();
}

void WorkspaceView::OnOptions()
{
   OptionsDLView o(this,m_workspace);
   o.exec();
}

void WorkspaceView::OnBufferListWindow()
{

    if(NULL == m_workspace->GetDataSetListView())
    {
        m_workspace->ShowDataSetListView();
    }
    else
    {
        m_workspace->ShowDataSetListView();
        QWidget* pw =(QWidget*)m_workspace->GetDataSetListView();
        m_workspace->WidgetAlert(pw);
    }
}


void WorkspaceView::OnSystemViewWindow()
{
   if(NULL == m_workspace->GetDeviceListView())
   {
       m_workspace->ShowSystemView();
   }
   else
   {
       m_workspace->ShowSystemView();
       QWidget* pw =(QWidget*)m_workspace->GetDeviceListView();
       m_workspace->WidgetAlert(pw);
   }
}

void WorkspaceView::OnLogWindow()
{
   if(NULL == m_workspace->GetLogView())
   {
       m_workspace->ShowLogView();
   }
   else
   {
       m_workspace->ShowLogView();
       QWidget* pw =(QWidget*)m_workspace->GetLogView();
       m_workspace->WidgetAlert(pw);
   }
}


void WorkspaceView::OnOpenScriptFileSelect()
{
   QString fileName = QFileDialog::getOpenFileName(this,tr("Open Script"), "", tr("JavaScript Files (*.js)"));
   if (!fileName.isNull())
   {
      OpenScriptFile(fileName);
   }
}

void WorkspaceView::OnOpenScriptFile()
{
   QString fileName = ((QAction*)sender())->data().toString();
   OpenScriptFile(fileName);
}

void WorkspaceView::OnFileNewMenu()
{
   m_menuFileNew->clear();

   DataClassType* script = m_workspace->GetDataClassManager().FindType(TERBIT_TYPE_SCRIPT_PROCESSOR);
   DataClassAction* pAct = m_workspace->GetActions()->makeProcessorCreateAction(script->GetId(), script->GetGraphics32(), m_menuFileNew);
   pAct->setText(script->GetDisplayName());
   m_menuFileNew->addAction(pAct);

   QMenu *pi = MakeDeviceTypeMenu(m_menuFileNew);
   if(NULL != pi)
   {
      pi->setTitle(tr("Device Instance"));
      m_menuFileNew->addMenu(pi);
   }

   pi = MakeDisplayTypeMenu(m_menuFileNew);
   if(NULL != pi)
   {
      pi->setTitle(tr("Display Instance"));
      m_menuFileNew->addMenu(pi);
   }

   pi = MakeProcessorTypeMenu(m_menuFileNew);
   if(NULL != pi)
   {
      pi->setTitle(tr("Processor Instance"));
      m_menuFileNew->addMenu(pi);
   }
}

void WorkspaceView::OnWindowMenu()
{
   QMenu* m = m_menuWindows;
   QAction* a;
   m->clear();
   a = new QAction(tr("&System View"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnSystemViewWindow()));
   m->addAction(a);
   a = new QAction(tr("&Data Sets"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnBufferListWindow()));
   m->addAction(a);
   a = new QAction(tr("&Log Viewer"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnLogWindow()));
   m->addAction(a);

   //show all open windows
   m->addSeparator();
   QList<WorkspaceDockWidget*> docks = findChildren<WorkspaceDockWidget*>();
   foreach(WorkspaceDockWidget* dock, docks)
   {
      a = new WindowAction(dock, this);
      a->setText(dock->windowTitle());
      connect(a, SIGNAL(triggered()),this, SLOT(OnMenuOpenWindowSelected()));
      m->addAction(a);
   }
}

void WorkspaceView::OnMenuOpenWindowSelected()
{
   WorkspaceDockWidget* w = ((WindowAction*)sender())->dockWidget;
   if (w->isHidden())
   {
      w->show();
   }
   w->raise();
   m_workspace->WidgetAlert(w);
}

void WorkspaceView::OnDeviceMenu()
{
    m_menuDevices->clear();

    QMenu *pi = MakeDeviceTypeMenu(m_menuDevices);
    if(NULL != pi)
    {
       //m->addMenu(pi);
       pi->setTitle(tr("Open new"));
       pi->setIcon(QIcon(":/images/32x32/add.png"));
       m_menuDevices->addMenu(pi);
    }
    else
    {
       //m_menuDevices->addMenu(m);
       // TODO: grey out and disable "Open new"
    }

    makeDevMenu(m_menuDevices);

}


// Creates a list of plugins, each item connected to a function
// That will create a new instance, if possible.
QMenu* WorkspaceView::MakeDeviceTypeMenu(QMenu* parent)
{
   QMenu* pMenu = new(std::nothrow) QMenu(parent);
   pMenu->setTitle("Available Devices");

   if(pMenu)
   {
      DataClassTypeMap::const_iterator it = m_workspace->GetDataClassTypes().begin();
      while(it != m_workspace->GetDataClassTypes().end())
      {
         DataClassType* t = it->second;
         if (t->GetKind() == DATA_CLASS_KIND_DEVICE)
         {
            DataClassAction* pAct = m_workspace->GetActions()->makeDevCreateAction(it->first, pMenu);
            // Change text from "Open" to the plugin name
            pAct->setText(t->GetDisplayName());
            pMenu->addAction(pAct);
         }
         ++it;
      }
   }
   return pMenu;
}

// Creates a list of devices, each device with a submenu of actions
void WorkspaceView::makeDevMenu(QMenu* parent)
{
   bool first = true;

   for(auto& i : m_workspace->GetDataClassManager().GetInstances())
   {
      DataClass* dc = i.second;
      if (dc->GetPublicScope() && dc->GetType()->GetKind() == DATA_CLASS_KIND_DEVICE)
      {
         if (first)
         {
            parent->addSeparator();
            first = false;
         }

         QMenu* pDev = new QMenu(dc->GetName(), parent);
         pDev->setIcon(QIcon(":/images/32x32/network_adapter.png"));
         DataClassAction* pAct = m_workspace->GetActions()->makeDevRemAction(dc->GetAutoId(), pDev);
         pDev->addAction(pAct);
         pAct = m_workspace->GetActions()->makeDevShowAction(dc->GetAutoId(), pDev);
         pDev->addAction(pAct);
         pAct = m_workspace->GetActions()->makeDevRenameAction(dc->GetAutoId(), pDev);
         pDev->addAction(pAct);

         parent->addMenu(pDev);

      }
   }
}
QMenu* WorkspaceView::MakeDisplayTypeMenu(QMenu* parent)
{
   QMenu* pMenu = new(std::nothrow) QMenu(parent);
   pMenu->setTitle("Available Displays");

   if(pMenu)
   {
      DataClassTypeMap::const_iterator it = m_workspace->GetDataClassTypes().begin();
      while(it != m_workspace->GetDataClassTypes().end())
      {
         DataClassType* t = it->second;
         if (t->GetKind() == DATA_CLASS_KIND_DISPLAY)
         {
            DataClassAction* pAct = m_workspace->GetActions()->makeDisplayCreateAction(it->first, t->GetGraphics32(), pMenu);
            pAct->setText(t->GetDisplayName());
            pMenu->addAction(pAct);
         }
         ++it;
      }
   }
   return pMenu;
}

QMenu *WorkspaceView::MakeProcessorTypeMenu(QMenu *parent)
{
   QMenu* pMenu = new(std::nothrow) QMenu(parent);
   pMenu->setTitle("Available Processors");

   if(pMenu)
   {
      DataClassTypeMap::const_iterator it = m_workspace->GetDataClassTypes().begin();
      while(it != m_workspace->GetDataClassTypes().end())
      {
         DataClassType* t = it->second;
         //script processor listed on other menu . . .
         if (t->GetKind() == DATA_CLASS_KIND_PROCESSOR && t->GetFullTypeName() != QString(TERBIT_TYPE_SCRIPT_PROCESSOR))
         {
            DataClassAction* pAct = m_workspace->GetActions()->makeProcessorCreateAction(it->first, t->GetGraphics32(), pMenu);
            pAct->setText(t->GetDisplayName());
            pMenu->addAction(pAct);
         }
         ++it;
      }
   }
   return pMenu;
}

void WorkspaceView::OpenScriptFile(const QString &fileName)
{
   m_workspace->OpenScriptFile(fileName);
   RebuildRecentScriptsMenu();
}

void WorkspaceView::OnClearScriptFileMRU()
{
   m_workspace->GetScriptFilesMRU().clear();
   RebuildRecentScriptsMenu();
}



void WorkspaceView::CreateMenus()
{
   QMenu* m;
   QAction* a;

   m = menuBar()->addMenu(tr("&File"));
   m_menuFileNew = new QMenu(tr("New"));
   connect(m_menuFileNew, SIGNAL(aboutToShow()), SLOT(OnFileNewMenu()));
   m->addMenu(m_menuFileNew);

   m->addSeparator();

   m_runScript = new QAction(tr("&Open Script . . ."),this);
   connect(m_runScript, SIGNAL(triggered()),this, SLOT(OnOpenScriptFileSelect()));
   m_runScript->setIcon(QIcon(":/images/32x32/script_lightning.png"));
   m->addAction(m_runScript);

   m_menuRecentScripts = m->addMenu(tr("Recent Scripts"));

   m->addSeparator();
   a = new QAction(tr("&Quit"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnQuit()));
   m->addAction(a);

   m_menuDevices = menuBar()->addMenu(tr("&Devices"));
   connect(m_menuDevices, SIGNAL(aboutToShow()), this, SLOT(OnDeviceMenu()));

   m_menuWindows = menuBar()->addMenu(tr("&Window"));
   connect(m_menuWindows, SIGNAL(aboutToShow()), this, SLOT(OnWindowMenu()));

   m = menuBar()->addMenu(tr("&Tools"));
   a = new QAction(tr("&Options"), this);
   a->setIcon(QIcon(":/images/32x32/setting_tools.png"));
   connect(a, SIGNAL(triggered()),this, SLOT(OnOptions()));
   m->addAction(a);

   m = menuBar()->addMenu(tr("&Help"));
   a = new QAction(tr("&Plugins"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnPlugins()));
   m->addAction(a);
   m->addSeparator();
   a = new QAction(tr("&About"), this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnAbout()));
   m->addAction(a);

}


void WorkspaceView::RebuildRecentScriptsMenu()
{
   QAction* a;
   m_menuRecentScripts->clear();
   ScriptFilesMRU& list = m_workspace->GetScriptFilesMRU();
   if (list.size() > 0)
   {
      ScriptFilesMRU::iterator it;
      for(it = list.begin(); it != list.end(); ++it)
      {
         QString display = *it;
         if (display.length() > 35)
         {
            display = QString("...").append(display.right(35));
         }
         a = new QAction(display, this);
         a->setData(*it);         
         a->setIcon(QIcon(":/images/32x32/script_lightning.png"));
         connect(a, SIGNAL(triggered()),this, SLOT(OnOpenScriptFile()));
         m_menuRecentScripts->addAction(a);
      }
      m_menuRecentScripts->addSeparator();
      a = new QAction(tr("Clear"), this);
      connect(a, SIGNAL(triggered()),this, SLOT(OnClearScriptFileMRU()));
      m_menuRecentScripts->addAction(a);
   }
}




}
