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
#include <QVBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QTextEdit>
#include <QSplitter>
#include <QFileInfo>
#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QShortcut>
#include <QAction>
#include <QMenu>
#include <QComboBox>
#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <tools/widgets/CodeEditor.h>
#include "ScriptProcessorView.h"
#include "ScriptProcessor.h"
#include "ScriptingUtils.h"
#include <connector-core/Workspace.h>
#include <connector-core/OptionsDL.h>

namespace terbit
{

ScriptProcessorView::ScriptProcessorView(ScriptProcessor *ide) : WorkspaceDockWidget(ide,ide->GetName()), m_ide(ide), m_loading(false)
{
   m_environmentDocumentation = BuildScriptEnvironmentDocumentation();

   QToolBar *tool = new QToolBar;
   QToolButton *button;

   tool->setMovable(false);

   QAction* a = new QAction(QIcon(":/images/32x32/folder.png"), tr("Open Script File"),this);
   connect(a, SIGNAL(triggered()),this, SLOT(OnFileOpen()));
   a->setShortcut(QKeySequence::Open);
   tool->addAction(a);

   m_save = new QAction(QIcon(":/images/32x32/disk.png"), tr("Save Script File"), this);
   tool->addAction(m_save);
   m_save->setShortcut(QKeySequence::Save);
   connect(m_save, SIGNAL(triggered()), this, SLOT(OnSave()));
   m_save->setEnabled(false);

   tool->addSeparator();

   button = new QToolButton;
   button->setText(tr("&Save As"));
   tool->addWidget(button);
   connect(button, SIGNAL(clicked()), this, SLOT(OnSaveAs()));
   tool->addSeparator();

   button = new QToolButton;
   button->setToolTip(tr("Execute (F5)"));
   button->setIcon(QIcon(":/images/32x32/lightning.png"));
   tool->addWidget(button);
   connect(button, SIGNAL(clicked()), this, SLOT(OnExecute()));
   m_execute = button;
   tool->addSeparator();


   button = new QToolButton;
   button->setPopupMode(QToolButton::InstantPopup);
   button->setText(tr("Documentation"));
   button->setToolTip(tr("Scripting documentation."));
   auto scriptlets = new QMenu(button);
   BuildScriptletMenu(scriptlets,false);
   button->setMenu(scriptlets);
   tool->addWidget(button);
   tool->addSeparator();

   button = new QToolButton;
   button->setToolTip(tr("Clear the scripting environment."));
   button->setIcon(QIcon(":/images/32x32/broom.png"));
   tool->addWidget(button);
   m_shutdown = button;
   m_shutdown->setEnabled(false);
   connect(button, SIGNAL(clicked()), this, SLOT(OnShutdownEngine()));

   tool->addSeparator();
   m_fontSize = new QComboBox();
   m_fontSize->addItem("8",8);
   m_fontSize->addItem("10",10);
   m_fontSize->addItem("12",12);
   m_fontSize->addItem("14",14);
   m_fontSize->addItem("16",16);
   m_fontSize->addItem("18",18);
   m_fontSize->addItem("24",24);
   m_fontSize->addItem("36",36);
   m_fontSize->setCurrentIndex(m_fontSize->findData(m_ide->GetWorkspace()->GetOptions()->GetDefaultFontSize()));
   connect(m_fontSize,SIGNAL(currentIndexChanged(int)),this,SLOT(OnFontSizeChange(int)));
   tool->addWidget(new QLabel(tr("Font Size")));
   tool->addWidget(m_fontSize);

   m_editor = new CodeEditor();
   m_messages = new QTextEdit();

   auto splitter = new QSplitter(Qt::Vertical);
   splitter->addWidget(m_editor);
   splitter->addWidget(m_messages);

   auto l = new QVBoxLayout();
   l->addWidget(tool);
   l->addWidget(splitter,1);

   QWidget* wrapper = new QWidget();
   wrapper->setLayout(l);
   setWidget(wrapper); //must use wrapper widget for dockwidget stuffs

   SetupKeyboardShortcuts();

   connect(m_editor,SIGNAL(undoAvailable(bool)),this, SLOT(OnUndoAvailable(bool)));

   connect(m_ide, SIGNAL(NameChanged(DataClass*)),this,SLOT(OnModelNameChanged(DataClass*)));
   connect(m_ide,SIGNAL(SourceCodeLoaded()),this, SLOT(OnModelSourceCodeLoaded()));
   connect(m_ide,SIGNAL(Print(QString,QString)),this, SLOT(OnModelPrint(QString,QString)));
   connect(m_ide,SIGNAL(PrintBr(QString,QString)),this, SLOT(OnModelPrintBr(QString,QString)));   
   connect(m_ide,SIGNAL(ExecutionStart()),this,SLOT(OnModelExecutionStart()));
   connect(m_ide,SIGNAL(ExecutionEnd()),this,SLOT(OnModelExecutionEnd()));
   connect(m_ide,SIGNAL(ScriptSaved()),this,SLOT(OnModelScriptSaved()));

   //init the data
   ApplyFontSize();
   OnModelSourceCodeLoaded();
}

ScriptProcessorView::~ScriptProcessorView()
{
   delete m_environmentDocumentation;
}

void ScriptProcessorView::ApplyFontSize()
{
   QFont font("monospace");
   font.setPointSize(m_fontSize->currentData().toInt());
   m_editor->setFont(font);
   m_messages->setFont(font);
}

QString ScriptProcessorView::GetFullSourceCode()
{
   return m_editor->toPlainText();
}

void ScriptProcessorView::OnExecute()
{
   QString code;
   if (m_editor->textCursor().hasSelection())
   {
      code = m_editor->textCursor().selectedText();
   }
   else
   {
      code = m_editor->toPlainText();
   }
   m_ide->Execute(code);
}

void ScriptProcessorView::OnFileOpen()
{
   bool ok = true;
   if (m_save->isEnabled())
   {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, tr("Save Script"), tr("Save Changes?"), QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes)
      {
         ok = SaveScript();
      }
   }

   if (ok)
   {
      QString dir = m_ide->GetFileName();
      if (dir.length() > 0)
      {
         QFileInfo info(dir);
         dir = info.absolutePath();
      }

      QString fileName = QFileDialog::getOpenFileName(this,tr("Open Script"), dir, tr("JavaScript Files (*.js)"));
      if (!fileName.isNull())
      {
         m_ide->LoadFile(fileName);
      }
   }
}

void ScriptProcessorView::OnSave()
{
   SaveScript();
}

void ScriptProcessorView::OnSaveAs()
{
   SaveScriptAs();
}

bool ScriptProcessorView::SaveScript()
{
   bool res;
   if (m_ide->HasFileName())
   {
      res = m_ide->Save(m_editor->toPlainText());
   }
   else
   {
      res = SaveScriptAs();
   }

   return res;
}

bool ScriptProcessorView::SaveScriptAs()
{
   bool res = false;
   QString dir = m_ide->GetFileName();
   if (dir.length() > 0)
   {
      QFileInfo info(dir);
      dir = info.absolutePath();
   }

   QString fileName = QFileDialog::getSaveFileName(this,tr("Save Script"), dir, tr("JavaScript Files (*.js)"));

   if (!fileName.isNull())
   {
      res = m_ide->SaveAs(fileName, m_editor->toPlainText());
   }

   return res;
}
void ScriptProcessorView::OnFontSizeChange(int)
{
   m_ide->GetWorkspace()->GetOptions()->SetDefaultFontSize(m_fontSize->currentData().toInt());
   ApplyFontSize();
}

void ScriptProcessorView::OnUndoAvailable(bool avail)
{
   m_save->setEnabled(avail);
}

void ScriptProcessorView::OnModelNameChanged(DataClass*)
{
   setWindowTitle(m_ide->GetName());
}

void ScriptProcessorView::OnModelSourceCodeLoaded()
{   
   m_editor->setPlainText(m_ide->GetSourceCode());
   m_save->setEnabled(false);
}

void ScriptProcessorView::OnModelPrint(const QString &html, const QString& colorName)
{
   QString h;
   if (colorName.length() > 0)
   {
      h = "<font color=\"" + colorName + "\"><label>" + html + "</label></font>";
   }
   else
   {
      h = "<label>" + html + "</label>";
   }
   m_messages->moveCursor(QTextCursor::End);
   m_messages->textCursor().insertHtml(h);
   m_messages->moveCursor(QTextCursor::End);
   m_messages->ensureCursorVisible();
}

void ScriptProcessorView::OnModelPrintBr(const QString &html, const QString &colorName)
{
   QString h;
   if (colorName.length() > 0)
   {
      h = "<font color=\"" + colorName + "\"><label>" + html + "<br/></label></font>";
   }
   else
   {
      h = "<label>" + html + "<br/></label>";
   }
   m_messages->moveCursor(QTextCursor::End);
   m_messages->textCursor().insertHtml(h);
   m_messages->moveCursor(QTextCursor::End);
   m_messages->ensureCursorVisible();
}

void ScriptProcessorView::OnModelExecutionStart()
{
   m_shutdown->setEnabled(false);
   m_execute->setEnabled(false);
}

void ScriptProcessorView::OnModelExecutionEnd()
{
   m_execute->setEnabled(true);
   m_shutdown->setEnabled(true);
}

void ScriptProcessorView::OnModelScriptSaved()
{
   m_save->setEnabled(false);
   m_editor->document()->clearUndoRedoStacks();
}


void ScriptProcessorView::SetupKeyboardShortcuts()
{
   QShortcut *s;

   s = new QShortcut(QKeySequence(Qt::Key_F5), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnExecute()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_S + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnSave()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_A + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnSaveAs()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_O + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnFileOpen()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Space + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnScriptletSense()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

}


void ScriptProcessorView::OnScriptletSense()
{
   QPoint globalPos = m_editor->viewport()->mapToGlobal(m_editor->cursorRect().topLeft());
   QMenu scriptletSense;
   BuildScriptletMenu(&scriptletSense,true);
   scriptletSense.exec(globalPos);
}

void ScriptProcessorView::OnShutdownEngine()
{
   m_ide->ShutdownEngine();
}

void ScriptProcessorView::BuildScriptletMenu(QMenu *menu, bool intellisense)
{
   auto instances = new QMenu(tr("Instances"),menu);
   instances->setProperty("intellisense",intellisense);
   connect(instances,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowInstances()));
   menu->addMenu(instances);

   auto types = new QMenu(tr("Types"),menu);
   types->setProperty("intellisense",intellisense);
   connect(types,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowTypes()));
   menu->addMenu(types);

   auto env = new EnvScriptletMenu(menu,m_environmentDocumentation,intellisense);
   connect(env,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowEnvScriptlets()));
   menu->addMenu(env);

   menu->addSeparator();

   if (intellisense)
   {
      auto print = new QAction(QIcon(":/images/32x32/script_yellow.png"),tr("Print"),menu);
      print->setData(intellisense);
      connect(print,SIGNAL(triggered()),this,SLOT(OnPrintScriptlet()));
      menu->addAction(print);

      auto printBr = new QAction(QIcon(":/images/32x32/script_yellow.png"),tr("PrintBr"),menu);
      printBr->setData(intellisense);
      connect(printBr,SIGNAL(triggered()),this,SLOT(OnPrintBrScriptlet()));
      menu->addAction(printBr);

      menu->addSeparator();
      auto fullRestore = new QAction(QIcon(":/images/32x32/script_yellow.png"),tr("Full Restore"),menu);
      fullRestore->setData(intellisense);
      connect(fullRestore,SIGNAL(triggered()),this,SLOT(OnFullRestoreScriptlet()));
      menu->addAction(fullRestore);
   }
   else
   {
      auto fullDoc = new QAction(QIcon(":/images/32x32/script_yellow.png"),tr("Full Documentation"),menu);
      fullDoc->setData(intellisense);
      connect(fullDoc,SIGNAL(triggered()),this,SLOT(OnFullDocumentationScriptlet()));
      menu->addAction(fullDoc);
   }

}

void ScriptProcessorView::OnMenuShowTypes()
{
   auto menu = ((QMenu*)sender());
   menu->clear();

   auto dataSources = new QMenu(tr("Data Sources"));
   dataSources->setIcon(QIcon(":/images/32x32/database_connect.png"));

   auto devices = new QMenu(tr("Devices"));
   devices->setIcon(QIcon(":/images/32x32/network_adapter.png"));

   auto processors = new QMenu(tr("Processors"));
   processors->setIcon(QIcon(":/images/32x32/function.png"));

   auto displays = new QMenu(tr("Displays"));
   displays->setIcon(QIcon(":/images/32x32/linechart.png"));

   for(auto& it : m_ide->GetWorkspace()->GetDataClassManager().GetTypes())
   {
      DataClassType* type = it.second;
      QMenu* subMenu = NULL;
      switch (type->GetKind())
      {
      case DATA_CLASS_KIND_SOURCE:
         if (type->GetFullTypeName() == TERBIT_TYPE_DATASET)
         {
            subMenu = menu;
         }
         else
         {
            subMenu = dataSources;
         }
         break;
      case DATA_CLASS_KIND_DISPLAY:
         subMenu = displays;
         break;
      case DATA_CLASS_KIND_PROCESSOR:
         subMenu = processors;
         break;
      case DATA_CLASS_KIND_DEVICE:
         subMenu = devices;
         break;
      }

      TypeScriptletMenu* typeMenu = new TypeScriptletMenu(subMenu, type->GetGraphics32(), type->GetDisplayName(),type, menu->property("intellisense").toBool());
      connect(typeMenu,SIGNAL(aboutToShow()),this, SLOT(OnMenuShowTypeScriptlets()));
      subMenu->addMenu(typeMenu);
   }

   menu->addMenu(dataSources);
   menu->addMenu(devices);
   menu->addMenu(processors);
   menu->addMenu(displays);
}

void ScriptProcessorView::OnMenuShowTypeScriptlets()
{
   auto m = ((TypeScriptletMenu*)sender());
   m->clear();

   TypeScriptletAction* ts;

   if (m->intellisense)
   {
      ts = new TypeScriptletAction(m,m->dataClassType,ScriptBuilder::TYPE_CREATE,m->intellisense);
      connect(ts,SIGNAL(triggered()),this,SLOT(OnMenuTypeScriptlet()));
      m->addAction(ts);

      ts = new TypeScriptletAction(m,m->dataClassType,ScriptBuilder::TYPE_FULL_NAME,m->intellisense);
      connect(ts,SIGNAL(triggered()),this,SLOT(OnMenuTypeScriptlet()));
      m->addAction(ts);

      m->addSeparator();
   }

   for(auto s : m->dataClassType->GetScriptDocumentation()->GetScriptlets())
   {
      auto a = new CustomScriptletAction(m,s,m->intellisense);
      connect(a,SIGNAL(triggered()),this,SLOT(OnMenuCustomScriptlet()));
      m->addAction(a);
   }

   if (m->dataClassType->GetScriptDocumentation()->GetSubDocumentation().size() > 0)
   {
      m->addSeparator();
      for (auto subDoc: m->dataClassType->GetScriptDocumentation()->GetSubDocumentation())
      {
         auto sub = new EnvScriptletMenu(m,subDoc,m->intellisense);
         connect(sub,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowEnvScriptlets()));
         m->addMenu(sub);
      }
   }

   if (!m->intellisense)
   {
      m->addSeparator();
      auto fd = new TypeScriptletAction(m,m->dataClassType,ScriptBuilder::TYPE_DOCUMENTATION, m->intellisense);
      connect(fd,SIGNAL(triggered()),this,SLOT(OnMenuTypeScriptlet()));
      m->addAction(fd);
   }
}

void ScriptProcessorView::OnMenuTypeScriptlet()
{
   auto a = ((TypeScriptletAction*)sender());

   QString script = ScriptBuilder::GenerateScript(a->scriptlet,a->dataClassType);

   ApplyScriptlet(script,a->intellisense);
}

void ScriptProcessorView::OnMenuCustomScriptlet()
{
   auto a = ((CustomScriptletAction*)sender());

   QString script;
   if (a->intellisense)
   {
      script = a->scriptlet->GetCode();
   }
   else
   {
      script = ScriptBuilder::GenerateDocumentation(a->scriptlet);
   }

   ApplyScriptlet(script,a->intellisense);
}

void ScriptProcessorView::OnMenuShowEnvScriptlets()
{
   auto m = ((EnvScriptletMenu*)sender());
   m->clear();

   for(auto s : m->scriptDocumentation->GetScriptlets())
   {
      auto a = new CustomScriptletAction(m,s,m->intellisense);
      connect(a,SIGNAL(triggered()),this,SLOT(OnMenuCustomScriptlet()));
      m->addAction(a);
   }

   if (m->scriptDocumentation->GetSubDocumentation().size() > 0)
   {
      m->addSeparator();
      for (auto subDoc: m->scriptDocumentation->GetSubDocumentation())
      {
         auto sub = new EnvScriptletMenu(m,subDoc,m->intellisense);
         connect(sub,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowEnvScriptlets()));
         m->addMenu(sub);
      }
   }
}

void ScriptProcessorView::OnMenuScriptDoc()
{
   auto a = ((ScriptDocumentationAction*)sender());

   QString script = ScriptBuilder::GenerateDocumentation(a->scriptDocumentation);

   ApplyScriptlet(script,a->intellisense);
}

void ScriptProcessorView::OnFullRestoreScriptlet()
{
   auto a = ((QAction*)sender());

   QString script = ScriptBuilder::GenerateFullRestoreScript(m_ide->GetWorkspace());

   ApplyScriptlet(script,a->data().toBool());
}

void ScriptProcessorView::OnFullDocumentationScriptlet()
{
   auto a = ((QAction*)sender());

   QString script = ScriptBuilder::GenerateFullDocumentationScript(m_ide->GetWorkspace(), m_environmentDocumentation);

   ApplyScriptlet(script,a->data().toBool());
}

void ScriptProcessorView::OnPrintScriptlet()
{
   auto a = ((QAction*)sender());

   QString script = ScriptBuilder::GeneratePrintScript();

   ApplyScriptlet(script,a->data().toBool());

}

void ScriptProcessorView::OnPrintBrScriptlet()
{
   auto a = ((QAction*)sender());

   QString script = ScriptBuilder::GeneratePrintBrScript();

   ApplyScriptlet(script,a->data().toBool());

}

void ScriptProcessorView::OnMenuShowInstances()
{
   auto menu = ((QMenu*)sender());
   menu->clear();

   auto dataSets =  new QMenu(tr("Data Sets"));
   dataSets->setIcon(QIcon(":/images/32x32/database_green.png"));

   auto dataSources = new QMenu(tr("Data Sources"));
   dataSources->setIcon(QIcon(":/images/32x32/database_connect.png"));

   auto devices = new QMenu(tr("Devices"));
   devices->setIcon(QIcon(":/images/32x32/network_adapter.png"));

   auto processors = new QMenu(tr("Processors"));
   processors->setIcon(QIcon(":/images/32x32/function.png"));

   auto displays = new QMenu(tr("Displays"));
   displays->setIcon(QIcon(":/images/32x32/linechart.png"));

   for(auto& it : m_ide->GetWorkspace()->GetDataClassManager().GetInstances())
   {
      DataClass* dc = it.second;
      if (dc->GetPublicScope())
      {
         QMenu* subMenu;
         switch (dc->GetType()->GetKind())
         {
         case DATA_CLASS_KIND_SOURCE:
            if (dc->IsDataSet())
            {
               subMenu = dataSets;
            }
            else
            {
               subMenu = dataSources;
            }
            break;
         case DATA_CLASS_KIND_DISPLAY:
            subMenu = displays;
            break;
         case DATA_CLASS_KIND_PROCESSOR:
            subMenu = processors;
            break;
         case DATA_CLASS_KIND_DEVICE:
            subMenu = devices;
            break;
         }

         InstanceScriptletMenu* instanceMenu = new InstanceScriptletMenu(subMenu, dc->GetType()->GetGraphics32(), dc->GetName(),dc, menu->property("intellisense").toBool());
         connect(instanceMenu,SIGNAL(aboutToShow()),this, SLOT(OnMenuShowInstanceScriptlets()));
         subMenu->addMenu(instanceMenu);
      }
   }

   if (dataSets->actions().count() > 0)
   {
      menu->addMenu(dataSets);
   }
   else
   {
      delete dataSets;
   }
   if (dataSources->actions().count() > 0)
   {
      menu->addMenu(dataSources);
   }
   else
   {
      delete dataSources;
   }
   if (devices->actions().count() > 0)
   {
      menu->addMenu(devices);
   }
   else
   {
      delete devices;
   }
   if (processors->actions().count() > 0)
   {
      menu->addMenu(processors);
   }
   else
   {
      delete processors;
   }
   if (displays->actions().count() > 0)
   {
      menu->addMenu(displays);
   }
   else
   {
      delete displays;
   }
}

void ScriptProcessorView::OnMenuShowInstanceScriptlets()
{
   auto m = ((InstanceScriptletMenu*)sender());
   m->clear();

   InstanceScriptletAction* ia;
   if (m->intellisense)
   {
      ia = new InstanceScriptletAction(m,m->dataClass,ScriptBuilder::INSTANCE_FIND,m->intellisense);
      connect(ia,SIGNAL(triggered()),this,SLOT(OnMenuInstanceScriptlet()));
      m->addAction(ia);

      ia = new InstanceScriptletAction(m,m->dataClass,ScriptBuilder::INSTANCE_UNIQUE_ID,m->intellisense);
      connect(ia,SIGNAL(triggered()),this,SLOT(OnMenuInstanceScriptlet()));
      m->addAction(ia);

      m->addSeparator();
   }

   auto t = m->dataClass->GetType();
   for(auto s : t->GetScriptDocumentation()->GetScriptlets())
   {
      auto a = new CustomScriptletAction(m,s,m->intellisense);
      connect(a,SIGNAL(triggered()),this,SLOT(OnMenuCustomScriptlet()));
      m->addAction(a);
   }

   if (t->GetScriptDocumentation()->GetSubDocumentation().size() > 0)
   {
      m->addSeparator();
      for (auto subDoc: t->GetScriptDocumentation()->GetSubDocumentation())
      {
         auto sub = new EnvScriptletMenu(m,subDoc,m->intellisense);
         connect(sub,SIGNAL(aboutToShow()),this,SLOT(OnMenuShowEnvScriptlets()));
         m->addMenu(sub);
      }
   }

   m->addSeparator();
   if (m->intellisense)
   {
      ia = new InstanceScriptletAction(m,m->dataClass,ScriptBuilder::INSTANCE_RESTORE,m->intellisense);
      connect(ia,SIGNAL(triggered()),this,SLOT(OnMenuInstanceScriptlet()));
      m->addAction(ia);
   }
   else
   {
      auto fd = new TypeScriptletAction(m,t,ScriptBuilder::TYPE_DOCUMENTATION, m->intellisense);
      connect(fd,SIGNAL(triggered()),this,SLOT(OnMenuTypeScriptlet()));
      m->addAction(fd);
   }
}

void ScriptProcessorView::OnMenuInstanceScriptlet()
{
   auto a = ((InstanceScriptletAction*)sender());

   QString script = ScriptBuilder::GenerateScript(a->scriptlet,a->dataClass);

   ApplyScriptlet(script,a->intellisense);
}

void ScriptProcessorView::ApplyScriptlet(const QString &script, bool intellisense)
{
   if (script.length() > 0)
   {
      if (intellisense)
      {
         m_editor->textCursor().insertText(script);
      }
      else
      {
         OnModelPrintBr(script,"Green");
      }
   }
}


ScriptProcessorView::TypeScriptletMenu::TypeScriptletMenu(QWidget *parent, const QIcon &icon, const QString &text, DataClassType *type, bool forIntellisense)
   : QMenu(parent), dataClassType(type), intellisense(forIntellisense)
{
   setIcon(icon);
   setTitle(text);
}

ScriptProcessorView::CustomScriptletAction::CustomScriptletAction(QObject *parent, Scriptlet* s, bool forIntellisense)
   : QAction(parent), scriptlet(s), intellisense(forIntellisense)
{
   setText(s->GetName());
}

ScriptProcessorView::TypeScriptletAction::TypeScriptletAction(QObject *parent, const DataClassType *type, ScriptBuilder::TypeScriptlets s, bool forIntellisense)
   : QAction(parent), dataClassType(type), scriptlet(s), intellisense(forIntellisense)
{
   switch (s)
   {
   case ScriptBuilder::TYPE_CREATE:
      setText(tr("Create"));
      break;
   case ScriptBuilder::TYPE_FULL_NAME:
      setText(tr("Full Type Name"));
      break;
   case ScriptBuilder::TYPE_DOCUMENTATION:
      setText(tr("%1 Documentation").arg(type->GetDisplayName()));
      break;
   }
}

ScriptProcessorView::InstanceScriptletMenu::InstanceScriptletMenu(QWidget *parent, const QIcon &icon, const QString &text, DataClass *dc, bool forIntellisense)
   : QMenu(parent), dataClass(dc), intellisense(forIntellisense)
{
   setIcon(icon);
   setTitle(text);
}

ScriptProcessorView::InstanceScriptletAction::InstanceScriptletAction(QObject *parent, DataClass *dc, ScriptBuilder::InstanceScriptlets s, bool forIntellisense)
   : QAction(parent), dataClass(dc), scriptlet(s), intellisense(forIntellisense)
{
   switch(s)
   {
   case ScriptBuilder::INSTANCE_FIND:
      setText(tr("Find"));
      break;
   case ScriptBuilder::INSTANCE_UNIQUE_ID:
      setText(tr("Unique Id"));
      break;
   case ScriptBuilder::INSTANCE_RESTORE:
      setText(tr("Restore"));
      break;
   }
}

ScriptProcessorView::EnvScriptletMenu::EnvScriptletMenu(QWidget *parent, ScriptDocumentation *d, bool forIntellisense)
   : QMenu(parent), scriptDocumentation(d), intellisense(forIntellisense)
{
   setTitle(d->GetName());
}

ScriptProcessorView::ScriptDocumentationAction::ScriptDocumentationAction(QObject *parent, ScriptDocumentation *doc, bool forIntellisense)
   : QAction(parent), scriptDocumentation(doc), intellisense(forIntellisense)
{
   setText(tr("Full Documentation"));

}





}
