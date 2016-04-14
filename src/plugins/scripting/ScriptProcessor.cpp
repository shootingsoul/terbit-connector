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
#include <QObject>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QFile>
#include <QJSEngine>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include "ScriptProcessor.h"
#include "ScriptProcessorView.h"
#include "ScriptDisplay.h"
#include <connector-core/LogDL.h>
#include <tools/Script.h>
#include <connector-core/Workspace.h>
#include "TerbitSW.h"
#include "ScriptingUtils.h"

namespace terbit
{

ScriptProcessor::ScriptProcessor() : m_view(NULL), m_scriptEngine(NULL), m_sourceProcessor(this)
{
}

ScriptProcessor::~ScriptProcessor()
{
   ShutdownEngine();

   //remove outputs owned by this
   for(auto& it : GetOutputs())
   {
      DataClass* dc = it.second;
      if (dc->GetOwner() == this)
      {
         GetWorkspace()->DeleteInstance(dc->GetAutoId());
      }
   }

   ClosePropertiesView();

   delete m_scriptEngine;
}

bool ScriptProcessor::Init()
{
   return true;
}

bool ScriptProcessor::InteractiveInit()
{
   ShowPropertiesView();
   return true;
}

void ScriptProcessor::SetSourceProcessor(ScriptProcessor *source)
{
   //connect print events to source processor view . . . so sub script runs display prints
   ScriptProcessorView* view = source->m_view;
   if (view)
   {
      connect(this,SIGNAL(Print(QString,QString)),view,SLOT(OnModelPrint(QString,QString)));
      connect(this,SIGNAL(PrintBr(QString,QString)),view,SLOT(OnModelPrintBr(QString,QString)));
   }
   m_sourceProcessor = source;
}

bool ScriptProcessor::ShowPropertiesView()
{
   if (m_view)
   {
      m_view->raise();
   }
   else
   {
      m_view = new ScriptProcessorView(this);
      connect(m_view,SIGNAL(destroyed()),this,SLOT(OnPropertiesViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);
      m_view->setFocus();
   }
   return true;
}

void ScriptProcessor::ClosePropertiesView()
{
   if (m_view)
   {
      m_view->close();
      m_view = NULL;
   }
}

void ScriptProcessor::GetDirectDependencies(std::list<DataClass *> &dependsOn)
{
   //??? how to determine dependencies???
}

void ScriptProcessor::Refresh()
{
   Execute(ActiveSourceCode());
}

void ScriptProcessor::LoadFile(const QString &fileName)
{
   m_fileName = fileName;
   QFileInfo info(m_fileName);
   SetName(info.fileName());
   GetWorkspace()->GetScriptFilesMRU().Add(fileName);
   LoadSourceCode(ReadFileSourceCode());
}

bool ScriptProcessor::AddScriptProcessorOutput(BlockIOCategory_t outputId, DataClass *output)
{
   return AddOutput(outputId, output);
}

void ScriptProcessor::LoadSourceCode(const QString &sourceCode)
{
   m_sourceCode = sourceCode;
   emit SourceCodeLoaded();
}

QString ScriptProcessor::ReadFileSourceCode()
{
   QString code;

   if (m_fileName.length() > 0)
   {
      QFile f(m_fileName);
      if (!f.open(QIODevice::ReadOnly))
      {
          LogError2(GetType()->GetLogCategory(),GetName(), QObject::tr("Script file not found: %1").arg(m_fileName));
      }
      else
      {
         QTextStream stream(&f);
         code = stream.readAll();
         f.close();
      }
   }

   return code;
}

QString ScriptProcessor::ActiveSourceCode()
{
   if (m_view)
   {
      return m_view->GetFullSourceCode();
   }
   else
   {
      return m_sourceCode;
   }
}

void ScriptProcessor::PerformScriptExecution(const QString& script)
{   
   bool startup = false;
   QString errorMessage;
   QString source = GetName();
   if (m_scriptEngine == NULL)
   {
      m_scriptEngine = new QJSEngine();
      ScriptingInitEngine(m_scriptEngine,this,GetWorkspace());
      startup = true;
   }
   emit ExecutionStart();
   auto res = ScriptRun(m_scriptEngine,script, source, errorMessage);
   emit ExecutionEnd();
   if (!res)
   {
      LogError2(GetType()->GetLogCategory(),GetName(),errorMessage);
      emit PrintBr(errorMessage, "Red");
   }
   if (startup)
   {
      emit EngineStartup();
   }
}


void ScriptProcessor::Execute(const QString &script)
{
   if (script.length() > 0)
   {
      PerformScriptExecution(script);
   }
}

bool ScriptProcessor::Save(const QString &script)
{
   bool res = false;

   m_sourceCode = script;

   if (m_fileName.length() > 0)
   {
      QFile f(m_fileName);
      if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
      {
          LogError2(GetType()->GetLogCategory(),GetName(), QObject::tr("Save script failed.  Script file couldn't not be open to write to: %1").arg(m_fileName));
      }
      else
      {
         QTextStream stream(&f);
         stream << script;
         f.close();
         res = true;
         emit ScriptSaved();
      }
   }
   else
   {
      res = true; // memory-based script
   }

   return res;
}

bool ScriptProcessor::SaveAs(const QString &fileName, const QString &script)
{
   m_fileName = fileName;
   QFileInfo info(m_fileName);
   SetName(info.fileName());
   return Save(script);
}



QVariant ScriptProcessor::ShowDialog(const QString &inputMatch, const QString& message, ScriptDialogView::Style style)
{
   QVariant res;

   ScriptDialogView v(NULL,style,message,inputMatch);
   res = v.ExecResult();

   return res;
}

void ScriptProcessor::OnPropertiesViewClosed()
{
   m_view = NULL;
}

void ScriptProcessor::OnBeforeDeleteOwner(DataClass *dc)
{
   Block::OnBeforeDeleteOwner(dc);
   //remove ourself
   GetWorkspace()->DeleteInstance(this->GetAutoId());
}

void ScriptProcessor::ShutdownEngine()
{
   if (m_scriptEngine)
   {      
      emit EngineShutdown();
      delete m_scriptEngine;
      m_scriptEngine = NULL;
   }
}

QObject *ScriptProcessor::CreateScriptWrapper(QJSEngine *se)
{
   return new ScriptProcessorSW(se,this);
}

void ScriptProcessor::BuildRestoreScript(ScriptBuilder &script, const QString &variableName)
{
   if (m_fileName.length() > 0)
   {
      script.add(QString("%1.LoadFile(%2);").arg(variableName).arg(ScriptEncode(m_fileName)));
   }
   else
   {
      script.add(QString("%1.LoadSourceCode(%2);").arg(variableName).arg(ScriptEncode(ActiveSourceCode())));
   }
   //do NOT run script by default
   //script.add(QString("%1.Refresh();").arg(variableName)); //executes the script code to initialize it

   if (m_view)
   {
      script.add(QString("%1.ShowPropertiesWindow();").arg(variableName));
   }
}


ScriptDocumentation *BuildScriptDocumentationScriptProcessor()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("JavaScript engine to process scripts."));

   d->AddScriptlet(new Scriptlet(QObject::tr("LoadFile"), "LoadFile(fileName);",QObject::tr("Load fully pathed file name to JavaScript code file.  The code is NOT executed.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("LoadSourceCode"), "LoadSourceCode(code);",QObject::tr("Load JavaScript source code.  The code is NOT executed.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Execute"), "Execute();",QObject::tr("Run the loaded code through the scripting engine.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Print"), "Print(html, optionalColorName);",QObject::tr("Print html to the output area with an optional color.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("PrintBr"), "PrintBr(html, optionalColorName);",QObject::tr("Print html to the output area with an optional color.  Adds the <br/> tag to the end of the html.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ClearContext"), "ClearContext();",QObject::tr("Clear the context or environment that the script runs in.  This removes all globally defined (in the scope of the script processor) variables and state.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("AddOutput"), "AddOutput(outputId, instance);",QObject::tr("Add an output instance with the given outputId.  The outputId is an integer that must be constant.  The instance is a reference variable or a unique id string.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("ShowDialog"), "ShowDialog(html);",QObject::tr("Displays a dialog HTML message.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowYesNoDialog"), "ShowYesNoDialog(html);",QObject::tr("Displays a dialog HTML message with Yes and No buttons.  Returns true if the yes button is pressed.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowOkCancelDialog"), "ShowOkCancelDialog(html);",QObject::tr("Displays a dialog HTML message with Ok and Cancel buttons.  Returns true if the Ok button is pressed.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowInputDialog"), "ShowInputDialog(html);",QObject::tr("Displays a dialog HTML message with an input field.  The input string is returned.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowConfirmDialog"), "ShowConfirmDialog(inputMatch, optionalHTMLMessage);",QObject::tr("Show a dialog and force the user to the enter the input match string.")));

   return d;
}

ScriptProcessorSW::ScriptProcessorSW(QJSEngine *se, ScriptProcessor *ide) : BlockSW(se,ide), m_scriptEngine(se), m_ide(ide)
{
   connect(ide,SIGNAL(EngineStartup()),this,SLOT(OnEngineStartup()));
   connect(ide,SIGNAL(EngineShutdown()),this,SLOT(OnEngineShutdown()));
}

void ScriptProcessorSW::ClearContext()
{
   m_ide->ShutdownEngine();
}

void ScriptProcessorSW::LoadFile(const QString &fileName)
{
   m_ide->LoadFile(fileName);
}

void ScriptProcessorSW::LoadSourceCode(const QString &sourceCode)
{
   m_ide->LoadSourceCode(sourceCode);
}

void ScriptProcessorSW::Print(const QString &html, const QString& colorName)
{
   emit m_ide->Print(html, colorName);
}

void ScriptProcessorSW::PrintBr(const QString &html, const QString& colorName)
{
   emit m_ide->PrintBr(html, colorName);
}

void ScriptProcessorSW::Execute()
{
   m_ide->Execute(m_ide->ActiveSourceCode());
}

bool ScriptProcessorSW::AddOutput(double outputId, const QJSValue &instance)
{
   bool res = false;
   DataClass* dc = m_dataClass->GetWorkspace()->FindInstance(instance);
   if (dc)
   {
      res = static_cast<ScriptProcessor*>(m_dataClass)->AddScriptProcessorOutput(outputId, dc);
   }
   else
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(), m_dataClass->GetName(),tr("AddOutput error.  The instance to add was not found."));
   }

   return res;
}

void ScriptProcessorSW::ShowDialog(const QString &message)
{
   m_ide->ShowDialog("",message,ScriptDialogView::STYLE_OK);
}

QJSValue ScriptProcessorSW::ShowInputDialog(const QString &message)
{
   QJSValue res(m_ide->ShowDialog("",message,ScriptDialogView::STYLE_INPUT).toString());
   return res;
}

QJSValue ScriptProcessorSW::ShowYesNoDialog(const QString &message)
{
   QJSValue res(m_ide->ShowDialog("",message,ScriptDialogView::STYLE_YESNO).toBool());
   return res;
}

QJSValue ScriptProcessorSW::ShowOkCancelDialog(const QString &message)
{
   QJSValue res(m_ide->ShowDialog("",message,ScriptDialogView::STYLE_OKCANCEL).toBool());
   return res;
}

void ScriptProcessorSW::OnEngineStartup()
{
   emit Startup();
}

void ScriptProcessorSW::OnEngineShutdown()
{
   emit Shutdown();
}

void ScriptProcessorSW::ShowConfirmDialog(const QJSValue& input, const QString& message)
{
   QString inputString = QJSValueToString(input);
   m_ide->ShowDialog(inputString,message,ScriptDialogView::STYLE_CONFIRM);
}




}
