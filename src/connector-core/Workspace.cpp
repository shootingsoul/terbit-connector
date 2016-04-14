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

#include "Workspace.h"
#include "WorkspaceView.h"
#include "WorkspaceDockWidget.h"
#include "DataSetListView.h"
#include "SystemView.h"
#include "BuildInfoCore.h"
#include "OptionsDL.h"
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include <list>
#include <QTime>
#include "LogDL.h"
#include "tools/Script.h"
#include "IDataClassFactory.h"
#include "LogView.h"
#include <QQmlContext>
#include <set>
#include "DataClassManager.h"
#include <plugins/scripting/ScriptProcessor.h>


namespace terbit
{

Workspace::Workspace(OptionsDL* options) : m_options(options), m_view(NULL), m_dataSetListView(NULL), m_systemView(NULL), m_logView(NULL), m_scriptFilesMRU(this)
{   
   m_dataClassManager = new DataClassManager();
   m_actions = new Actions(this);
   m_bufferType = NULL;

   connect(this, SIGNAL(ShowDataSetListView()),this, SLOT(OnShowDataSetListView()));
   connect(this, SIGNAL(CloseDataSetListView()),this, SLOT(OnCloseDataSetListView()));
   connect(this, SIGNAL(ShowSystemView()),this, SLOT(OnShowSystemView()));
   connect(this, SIGNAL(CloseSystemView()),this, SLOT(OnCloseSystemView()));
   connect(this, SIGNAL(ShowLogView()),this, SLOT(OnShowLogView()));
   connect(this, SIGNAL(CloseLogView()),this, SLOT(OnCloseLogView()));
   connect(this, SIGNAL(DeleteInstanceThreaded(DataClassAutoId_t)),this,SLOT(DeleteInstance(DataClassAutoId_t)));

}

Workspace::~Workspace()
{
   //make sure GUI is gone
   delete m_view;
   m_view = NULL;

   //ensure data class objects are destroyed before the rest of the workspace objects are automatically destructed
   delete m_dataClassManager;

   //plugins are managed here so remove them
   for(PluginList::iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
   {
      delete (*i);
   }
}

void Workspace::Startup(QApplication& app)
{
   //in case restore script or anything else crashed program . . . we won't run it again on next startup
   //track clean starts in options file . . . written to write away
   bool lastStartClean = m_options->GetLastStartClean();
   m_options->SetLastStartClean(false);

   if (m_options->GetDarkTheme())
   {
      ApplyDarkTheme();
   }

   LoadPlugins();
   LoadTranslations(app);
   ShowView();

   RestoreSession(lastStartClean);
   //made it through start cleanly . . . yay
   m_options->SetLastStartClean(true);

}

void Workspace::ShowView()
{
   m_view = new WorkspaceView(this);
   m_view->show();   
}

void Workspace::RestoreSession(bool lastStartClean)
{
   QString script;
   m_options->GetRestoreSessionValues(script);

   dvar(script);

   if (script.length() == 0 || !(m_options->GetRestoreSessionEnabled()) || !lastStartClean )
   {
      if (!lastStartClean)
      {
         //this script failed on last startup . . . log it
         LogWarning(g_log.startup,tr("The last restore script crashed the program.  Here is the script.\n\n%1").arg(script));
         dvar(script);
      }

      //defaults      
      m_view->resize(1200,750);
      m_view->move(50,50);      
      ShowDataSetListView();
      ShowSystemView();
   }
   else
   {
      //try our best to restore state . . .
      RunScript(script,"Startup::RestoreState", NULL);

      //NOTE: for some unknown reason the restoreState doesn't always restore the hidden flag properly (i.e. should be showing but are hidden)
      //so we'll force all dock widgets to show and assume non dock widgets should be hidden.  Somewhat limiting, but c'est la vie
      //also noticed that forcing the show doesn't restore to proper position, but at least it's showing again . . .
      QList<WorkspaceDockWidget*> docks = m_view->findChildren<WorkspaceDockWidget*>();
      foreach(WorkspaceDockWidget* dock, docks)
      {
         if (dock->isHidden())
         {
            dock->show();
         }
      }

   }
}

void Workspace::RestoreSessionAssignObjectNames()
{
   //Ensure all dock widgets have a unique object name
   //must have ObjectName set for dock windows to restore them properly
   QList<WorkspaceDockWidget*> docks = m_view->findChildren<WorkspaceDockWidget*>();
   foreach(WorkspaceDockWidget* dock, docks)
   {
      dock->AssignUniqueObjectName();
   }
}

void Workspace::SaveRestoreSessionState()
{
   if (m_options->GetRestoreSessionEnabled())
   {
      QString script = ScriptBuilder::GenerateFullRestoreScript(this);

      m_options->SetRestoreSessionValues(script);
   }
}

void Workspace::WidgetAlert(QWidget *w)
{
   QPropertyAnimation *anim = new QPropertyAnimation(w, "pos");
   QPoint p = w->pos();

   anim->setKeyValueAt(0, p);
   anim->setKeyValueAt(1, p);
   p.setY(p.y() - 20);
   anim->setKeyValueAt(.5, p);

   anim->setDuration(1000);

   anim->setEasingCurve(QEasingCurve::OutBounce);
   anim->setLoopCount(1);
   anim->start();
}

void Workspace::AddDockWidget(WorkspaceDockWidget* w)
{
   w->setParent(m_view);

   if (w->GetOrigin() != NULL)
   {
      switch (w->GetOrigin()->GetType()->GetKind())
      {
         case DATA_CLASS_KIND_DISPLAY:
            m_view->addDockWidget(Qt::RightDockWidgetArea,w);
            break;
         default:
            m_view->addDockWidget(Qt::LeftDockWidgetArea,w);
            break;
      }
   }
   else
   {
      m_view->addDockWidget(Qt::LeftDockWidgetArea,w);
   }
}

void Workspace::RemDataClassDocks(DataClass *d)
{
   QList<WorkspaceDockWidget*> docks = m_view->findChildren<WorkspaceDockWidget*>();
   for(WorkspaceDockWidget *w: docks)
   {
      if(w->GetOrigin() == d)
      {
         w->close();
      }
   }
}

void Workspace::FloatDockWidgetOutside(WorkspaceDockWidget *w)
{
   w->setFloating(true);
   w->move(m_view->x()+m_view->width()+50,m_view->y()+50);
   w->update();
}

void Workspace::OnShowDataSetListView()
{
   if (m_dataSetListView == NULL)
   {
      m_dataSetListView = new DataSetListView(this);
      m_dataSetListView->setParent(m_view);
      m_dataSetListView->show();
      connect(m_dataSetListView,SIGNAL(destroyed()),this,SLOT(OnDataSetListViewClosed()));
      m_view->addDockWidget(Qt::LeftDockWidgetArea, m_dataSetListView);
      if (m_systemView)
      {
         m_view->tabifyDockWidget(m_systemView,m_dataSetListView);
      }
      else if (m_logView)
      {
         m_view->tabifyDockWidget(m_logView,m_dataSetListView);
      }
      m_dataSetListView->raise();
   }
   else
   {
      m_dataSetListView->raise();
      WidgetAlert(m_dataSetListView);
   }
}

void Workspace::OnCloseDataSetListView()
{
   if (m_dataSetListView)
   {
      disconnect(m_dataSetListView,SIGNAL(destroyed()),this,SLOT(OnDataSetListViewClosed()));
      delete m_dataSetListView;
      m_dataSetListView = NULL;
   }
}

void Workspace::OnDataSetListViewClosed()
{
   m_dataSetListView = NULL;
}


void Workspace::OnShowLogView()
{

   if (m_logView == NULL)
   {      
      m_logView = new LogView(this,&g_logManager);
      m_logView->setParent(m_view);
      m_logView->show();
      connect(m_logView,SIGNAL(destroyed()),this,SLOT(OnLogViewClosed()));
      m_view->addDockWidget(Qt::LeftDockWidgetArea, m_logView);
      if (m_systemView)
      {
         m_view->tabifyDockWidget(m_systemView,m_logView);
      }
      else if (m_dataSetListView)
      {
         m_view->tabifyDockWidget(m_dataSetListView,m_logView);
      }
   }
   m_logView->raise();
}

void Workspace::OnCloseLogView()
{
   if (m_logView)
   {
      disconnect(m_logView,SIGNAL(destroyed()),this,SLOT(OnLogViewClosed()));
      delete m_logView;
      m_logView = NULL;
   }

}

void Workspace::OnLogViewClosed()
{
   m_logView = NULL;
}

void Workspace::ApplyDarkTheme()
{
   qApp->setStyle(QStyleFactory::create("Fusion"));

   QPalette darkPalette;
   darkPalette.setColor(QPalette::Window, QColor(53,53,53));
   darkPalette.setColor(QPalette::WindowText, Qt::white);
   darkPalette.setColor(QPalette::Base, QColor(25,25,25));
   darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
   darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
   darkPalette.setColor(QPalette::ToolTipText, Qt::white);
   darkPalette.setColor(QPalette::Text, Qt::white);
   darkPalette.setColor(QPalette::Button, QColor(53,53,53));
   darkPalette.setColor(QPalette::ButtonText, Qt::white);
   darkPalette.setColor(QPalette::BrightText, Qt::red);
   darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

   darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
   darkPalette.setColor(QPalette::HighlightedText, Qt::black);

   qApp->setPalette(darkPalette);

   qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}


DataSet *Workspace::CreateDataSet(DataClass *owner, bool publicScope)
{
   return static_cast<DataSet*>(CreateInstance(m_bufferType, owner, publicScope));
}

DataClass* Workspace::CreateInstance(DataClassAutoId_t typeId, DataClass* owner, bool publicScope)
{
   DataClass* d = NULL;
   DataClassType* t = m_dataClassManager->FindType(typeId);

   if (t != NULL)
   {
      d = CreateInstance(t, owner, publicScope);
   }
   else
   {
      LogError(g_log.general,tr("CreateObject typeId not found."));
   }
   return d;
}

DataClass* Workspace::CreateInstance(DataClassType* type, DataClass* owner, bool publicScope)
{
   DataClass* d = m_dataClassManager->Create(type);
   if (d)
   {
      m_dataClassManager->Add(d,owner,type,this, publicScope); //sets id
      if (d->Init())
      {
         emit InstanceCreated(d);
      }
      else
      {
         DeleteInstance(d->GetAutoId());
         d = NULL;
      }
   }
   return d;
}

DataClass* Workspace::CreateInstance(const QString& fullTypeName, DataClass* owner, bool publicScope)
{
   DataClass* res = NULL;
   for(DataClassTypeMap::const_iterator i = GetDataClassTypes().begin(); i != GetDataClassTypes().end(); i++)
   {
      DataClassType* t = i->second;
      if (QString::compare(t->GetFullTypeName(), fullTypeName, Qt::CaseInsensitive) == 0)
      {
         res = CreateInstance(t,owner, publicScope);
         break;
      }
   }
   return res;
}

DataClass *Workspace::FindInstance(DataClassAutoId_t id)
{
    return m_dataClassManager->Find(id);
}
DataClass *Workspace::FindInstance(const QString& uniqueId)
{
   return m_dataClassManager->Find(uniqueId);
}

DataClass *Workspace::FindInstance(const QJSValue &value)
{
   DataClass* dc = NULL;
   if (value.isQObject())
   {
      DataClassSW* psw = static_cast<DataClassSW*>(value.toQObject());
      if (psw)
      {
         dc = psw->GetDataClass();
      }
   }
   else if (value.isString())
   {
      dc = GetDataClassManager().Find(value.toString());
   }

   return dc;
}


void Workspace::OnShowSystemView()
{
   if (m_systemView == NULL)
   {
      m_systemView = new SystemView(this);
      m_systemView->setParent(m_view);
      m_systemView->show();
      connect(m_systemView,SIGNAL(destroyed()),this,SLOT(OnSystemViewClosed()));
      m_view->addDockWidget(Qt::LeftDockWidgetArea, m_systemView);
      if (m_dataSetListView)
      {
         m_view->tabifyDockWidget(m_dataSetListView, m_systemView);
      }
      else if (m_logView)
      {
         m_view->tabifyDockWidget(m_logView, m_systemView);
      }

      m_systemView->raise();
   }
   else
   {
      m_systemView->raise();
      WidgetAlert(m_systemView);
   }
}

void Workspace::OnCloseSystemView()
{
   if (m_systemView)
   {
      disconnect(m_systemView,SIGNAL(destroyed()),this,SLOT(OnSystemViewClosed()));
      delete m_systemView;
      m_systemView = NULL;
   }
}

void Workspace::OnSystemViewClosed()
{
   m_systemView = NULL;
}

// This function called when the app is shutdown via workspace view
void Workspace::ShutdownPrep()
{
   //prepare to shutdown
   SaveRestoreSessionState(); //so we can reload to how we are now

   // Set view to NULL so in Destructor, we can delete if not NULL
   // programmatic destruction
   m_view = NULL;
}

void Workspace::DeleteInstance(DataClassAutoId_t id)
{
   m_dataClassManager->Delete(id);
}


bool Workspace::RenameInstance(DataClassAutoId_t id, const QString& str)
{
   bool retVal = false;

   DataClass* d = m_dataClassManager->Find(id);

   if(d)
   {
      // TODO: qualify name
      d->SetName(str);
      retVal = true;
   }
   return retVal;
}


void Workspace::LoadPlugins(void)
{

   QString internalVersion = QString("%1 %2").arg(_STR_PRODUCT_VERSION).arg(CompilerOptions());
   QString internalFile  = QObject::tr("Internal");
   Plugin* p;

   //core factory   
   p = new Plugin(m_coreFactory.GetName(), true, internalFile,"",internalVersion, m_coreFactory.GetProviderName(), m_coreFactory.GetDescription());
   m_plugins.push_back(p);
   m_dataClassManager->AddTypes(p,&m_coreFactory);

   for(DataClassTypeMap::const_iterator i = m_dataClassManager->GetTypes().begin(); i != m_dataClassManager->GetTypes().end(); ++i )
   {
      if ((i->second)->GetFullTypeName() == TERBIT_TYPE_DATASET)
      {
         m_bufferType = i->second;
         break;
      }
   }

   //static plugins
   IDataClassFactory* idcf = NULL;
   foreach (QObject *factory, QPluginLoader::staticInstances())
   {
      idcf = qobject_cast<IDataClassFactory *>(factory);
      if (idcf)
      {
         p = new Plugin(idcf->GetName(), true, internalFile,"",internalVersion, idcf->GetProviderName(),idcf->GetDescription());
         m_plugins.push_back(p);
         m_dataClassManager->AddTypes(p,idcf);
      }
   }

   //external plugins
   QDir pluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
   if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
   {
       pluginsDir.cdUp();
   }
#elif defined(Q_OS_MAC) // in case we ever port to Mac
   if (pluginsDir.dirName() == "MacOS") {
       pluginsDir.cdUp();
       pluginsDir.cdUp();
       pluginsDir.cdUp();
   }
#endif

   pluginsDir.cd("plugins");

   foreach (QString fileName, pluginsDir.entryList(QDir::Files))
   {
      QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));

      QObject *factory = loader.instance();      
      if (factory)
      {
         QString version = loader.metaData().value("MetaData").toObject().value("version").toString();
         QString provider;

         idcf = qobject_cast<IDataClassFactory *>(factory);
         if (idcf)
         {
            provider = idcf->GetProviderName();
            p = new Plugin(idcf->GetName(),false, fileName, pluginsDir.absolutePath(),version, provider,idcf->GetDescription());
            m_plugins.push_back(p);
            m_dataClassManager->AddTypes(p,idcf);
         }                  
      }
      else
      {
         // TODO: log error
         QString errStr = loader.errorString();
      }
   }
}

void Workspace::LoadTranslations(QApplication& app)
{
   auto qtTranslator = new QTranslator(&app);
   if (qtTranslator->load("qt_" + QLocale::system().name(),QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
   {
      app.installTranslator(qtTranslator);
   }

   QDir translationsDir;
   QLocale systemLocale = QLocale::system();
   //search plugins subdir
   translationsDir = QDir(qApp->applicationDirPath());
   if (translationsDir.cd("plugins"))
   {
      LoadTranslationsDir(app,translationsDir,systemLocale);
   }

   //search translations subdir
   translationsDir = QDir(qApp->applicationDirPath());
   if (translationsDir.cd("translations"))
   {
      LoadTranslationsDir(app,translationsDir,systemLocale);
   }
}

void Workspace::LoadTranslationsDir(QApplication& app, QDir& dir, QLocale& locale)
{
   //trans file to use may not match system setting exactly . . .
   //build list of base names to use so we can let loader pick proper trans file based on locale

   std::set<QString> baseNames;
   QStringList filters;
   filters << "*.qm";

   foreach (QString fileName, dir.entryList(filters, QDir::Files))
   {
      int n = fileName.lastIndexOf('.',-4); //skip over qm extension and language name
      if (n > 0)
      {
         QString base = fileName.left(n);
         baseNames.insert(base); //set ensures unique list
      }
   }

   for(auto& s : baseNames)
   {
      auto myappTranslator = new QTranslator(&app);
      if (myappTranslator->load(locale,s,".",dir.absolutePath()))
      {
         app.installTranslator(myappTranslator);
      }
   }
}


void Workspace::OpenScriptFile(const QString &fileName)
{
   m_scriptFilesMRU.Add(fileName);

   auto s = static_cast<ScriptProcessor*>(CreateInstance(TERBIT_TYPE_SCRIPT_PROCESSOR,NULL,true));
   if (s)
   {
      s->InteractiveInit();
      s->LoadFile(fileName);
   }
}

void Workspace::RunScript(const QString &script, const QString &source, ScriptProcessor* sourceProcessor)
{
   //run script through the script processor
   auto s = static_cast<ScriptProcessor*>(CreateInstance(TERBIT_TYPE_SCRIPT_PROCESSOR,NULL,false));
   if (s)
   {
      if (sourceProcessor)
      {
         s->SetSourceProcessor(sourceProcessor->GetSourceProcessor());
      }
      s->SetName(source);
      s->Execute(script);
      DeleteInstance(s->GetAutoId());
   }
}

void Workspace::BuildDockAreaRestoreScript(ScriptBuilder& script, const QString& variableName)
{
   script.add(tr("//Dock Restore"));
   if (m_dataSetListView)
   {
      script.add(QString("%1.ShowDataSetsWindow();").arg(variableName));
   }
   if (m_systemView)
   {
      script.add(QString("%1.ShowSystemViewWindow();").arg(variableName));
   }
   if (m_logView)
   {
      script.add(QString("%1.ShowLogViewerWindow();").arg(variableName));
   }
   RestoreSessionAssignObjectNames();
   script.add(QString("%1.AssignDockObjectNames();").arg(variableName));

   QByteArray b64;
   b64 = m_view->saveGeometry().toBase64();
   script.add(QString("%1.DockRestoreGeometry(%2);").arg(variableName).arg(ScriptEncode(QString(b64))));
   b64 = m_view->saveState().toBase64();
   script.add(QString("%1.DockRestoreState(%2);").arg(variableName).arg(ScriptEncode(QString(b64))));
}

void Workspace::DockRestoreGeometry(const QString &geometry)
{
   if (m_view)
   {
      QByteArray b64;
      b64.append(geometry);
      m_view->restoreGeometry(QByteArray::fromBase64(b64));
   }
}

void Workspace::DockRestoreState(const QString &state)
{
   if (m_view)
   {
      QByteArray b64;
      b64.append(state);
      m_view->restoreState(QByteArray::fromBase64(b64));
   }
}


ScriptDocumentation *BuildScriptDocumentationWorkspace()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->SetName(QObject::tr("Workspace"));
   d->SetSummary(QObject::tr("Functions to work the core application to manage instances, work standard windows, and run other scripts."));

   d->AddScriptlet(new Scriptlet(QObject::tr("ShowSystemViewWindow"), "ShowSystemViewWindow();",QObject::tr("Show the system view window to list the hierarchy of instances.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("CloseSystemViewWindow"), "CloseSystemViewWindow();",QObject::tr("Close the system view window to list the hierarchy of instances.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowDataSetsWindow"), "ShowDataSetsWindow();",QObject::tr("Show the window that lists all of the data sets.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("CloseDataSetsWindow"), "CloseDataSetsWindow();",QObject::tr("Close the window that lists all of the data sets.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowLogViewerWindow"), "ShowLogViewerWindow();",QObject::tr("Show the log viewer window.  This displays all logged events that occur after the window is open.  It does NOT events prior to opening the window.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("CloseLogViewerWindow"), "CloseLogViewerWindow();",QObject::tr("Close the log viewer window.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("AddDataSet"), "AddDataSet(optionalOwner, optionalPublicScope);",QObject::tr("Add a new data set and returns a reference to it.  The owner defaults to null, and the publicScope defaults to true.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("AddRemoteDataSet"), "AddRemoteDataSet(source, optionalOwner, optionalPublicScope);",QObject::tr("Adds a remote data set from an existing data source or data set and returns a reference to the new data set.  The source is a reference or unique id string.  The owner defaults to null, and the publicScope defaults to true.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Add"), "Add(fullTypeName, optionalOwner, optionalPublicScope);",QObject::tr("Adds a new instance based on the full type name string and returns a reference to it.  The owner defaults to null, and the publicScope defaults to true.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Remove"), "Remove(instance);",QObject::tr("Removes the instance.  The instance is a reference or unique id string.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Find"), "Find(uniqueId);",QObject::tr("Returns a reference to the instance with the given unique id string.  Undefined is returned if the unique id string is not found.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("RunScript"), "RunScript(sourceCode);",QObject::tr("Executes the JavaScript source code in another context.  Variables defined in the current context are not accesible in the new script context.  Communication may be done through instances in the workspace.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("RunScriptFile"), "RunScriptFile(fileName);",QObject::tr("Executes the JavaScript code from the file in another context.  The file name is fully pathed.  Variables defined in the current context are not accesible in the new script context.  Communication may be done through instances in the workspace.")));

   return d;
}

//dont' set parent on script wrapper so script engine will own it
WorkspaceSW::WorkspaceSW(QJSEngine* se, Workspace* w, ScriptProcessor* sourceProcessor)
   : QObject(), m_scriptEngine(se), m_workspace(w), m_sourceProcessor(sourceProcessor)
{
}

void WorkspaceSW::AssignDockObjectNames()
{
   m_workspace->RestoreSessionAssignObjectNames();
}

void WorkspaceSW::DockRestoreGeometry(const QString &geometry)
{
   m_workspace->DockRestoreGeometry(geometry);
}

void WorkspaceSW::DockRestoreState(const QString &state)
{
   m_workspace->DockRestoreState(state);
}

void WorkspaceSW::ShowDataSetsWindow()
{
   m_workspace->ShowDataSetListView();
}

void WorkspaceSW::ShowSystemViewWindow()
{
   m_workspace->ShowSystemView();
}

void WorkspaceSW::ShowLogViewerWindow()
{
   m_workspace->ShowLogView();;
}

void WorkspaceSW::CloseDataSetsWindow()
{
   m_workspace->CloseDataSetListView();
}

void WorkspaceSW::CloseSystemViewWindow()
{
   m_workspace->CloseSystemView();
}

void WorkspaceSW::CloseLogViewerWindow()
{
   m_workspace->CloseLogView();
}

QJSValue WorkspaceSW::AddDataSet(const QJSValue &parent, bool publicScope)
{
   QJSValue res;

   DataClass* p = m_workspace->FindInstance(parent);

   DataSet* d = m_workspace->CreateDataSet(p, publicScope);
   if (d)
   {
      res = m_scriptEngine->newQObject(d->CreateScriptWrapper(m_scriptEngine));
   }
   return res;
}

QJSValue WorkspaceSW::AddRemoteDataSet(const QJSValue& source, const QJSValue &owner, bool publicScope)
{
   QJSValue res;
   DataClass* src = m_workspace->FindInstance(source);
   DataClass* o = m_workspace->FindInstance(owner);

   if (src && src->GetType()->GetKind() == DATA_CLASS_KIND_SOURCE )
   {
      DataSet* newDs = CreateRemoteDataSet(static_cast<DataSource*>(src), o, publicScope);
      if (newDs)
      {
         res = m_scriptEngine->newQObject(newDs->CreateScriptWrapper(m_scriptEngine));
      }
   }
   else
   {
      LogError(g_log.general,tr("Invalid source parameter for add remote data set"));
   }
   return res;
}

QJSValue WorkspaceSW::Add(const QString &name, const QJSValue& parent, bool publicScope)
{
   QJSValue res;

   DataClass* p = m_workspace->FindInstance(parent);

   DataClass* d = m_workspace->CreateInstance(name, p, publicScope);
   if (d)
   {
      res = m_scriptEngine->newQObject(d->CreateScriptWrapper(m_scriptEngine));
   }
   return res;
}

void WorkspaceSW::Remove(const QJSValue &autoOrUniqueId)
{
   DataClass* d = m_workspace->FindInstance(autoOrUniqueId);

   if (d)
   {
      m_workspace->DeleteInstanceThreaded(d->GetAutoId());
   }
}


QJSValue WorkspaceSW::Find(const QJSValue& uniqueId)
{
   QJSValue res;

   DataClass* d = NULL;

   if (uniqueId.isString())
   {
      d = m_workspace->GetDataClassManager().Find(uniqueId.toString());
   }

   if (d)
   {
      res = m_scriptEngine->newQObject(d->CreateScriptWrapper(m_scriptEngine));
   }

   return res;
}

void WorkspaceSW::RunScript(const QString &sourceCode)
{   
   m_workspace->RunScript(sourceCode,m_sourceProcessor->GetName(), m_sourceProcessor);
}

void WorkspaceSW::RunScriptFile(const QString &fileName)
{
   QString sourceCode;
   QFile file(fileName);
   if(!file.open(QIODevice::ReadOnly))
   {
      LogError(g_log.general,tr("RunScriptFile couldn't open script file '%1'").arg(fileName));
   }
   else
   {
      QTextStream in(&file);
      sourceCode = in.readAll();
   }
   file.close();

   if (sourceCode.length() > 0)
   {
      m_workspace->RunScript(sourceCode, fileName, m_sourceProcessor);
   }
}

QJSValue WorkspaceSW::FindDock(const QString &objectName)
{
   QJSValue res;
   WorkspaceDockWidget* dock = m_workspace->m_view->findChild<WorkspaceDockWidget*>(objectName);
   if (dock != NULL)
   {
      res = m_scriptEngine->newQObject(new WorkspaceDockWidgetSW(m_scriptEngine,dock));
   }

   return res;
}

}//namespace terbit
