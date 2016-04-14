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

#include <list>
#include <QString>
#include <QObject>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <tools/Script.h>
#include <connector-core/Block.h>
#include <connector-core/Event.h>
#include "ScriptDialogView.h"

namespace terbit
{
class ScriptProcessorView;
class Workspace;
class ScriptProcessorThread;
class ScriptProcessorEventScriptlet;

static const char* SCRIPTPROCESSOR_TYPENAME = "processor"; //already under scripting namespace

class ScriptProcessor : public Block
{
   Q_OBJECT

   friend class ScriptProcessorThread;
public:
   ScriptProcessor();
   virtual ~ScriptProcessor();

   bool Init();
   bool InteractiveInit();

   ScriptProcessor* GetSourceProcessor() { return m_sourceProcessor; }
   void SetSourceProcessor(ScriptProcessor* source);

   QObject* CreateScriptWrapper(QJSEngine* se);
   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   const QString& GetFileName() { return m_fileName; }
   void LoadFile(const QString& fileName);
   bool HasFileName() { return m_fileName.length() > 0; }

   bool AddScriptProcessorOutput(BlockIOCategory_t outputId, DataClass* output);

   void LoadSourceCode(const QString& sourceCode);
   const QString& GetSourceCode() { return m_sourceCode; }

   void Execute(const QString& script);
   bool Save(const QString& script);
   bool SaveAs(const QString& fileName, const QString& script);

   void ShutdownEngine();

   QVariant ShowDialog(const QString &inputMatch, const QString& message, ScriptDialogView::Style style);

   bool ShowPropertiesView();
   void ClosePropertiesView();

   virtual void GetDirectDependencies(std::list<DataClass *> &dependsOn);

   virtual void Refresh();
   QString ActiveSourceCode();
signals:
   void ScriptSaved();
   void SourceCodeLoaded();
   void EngineShutdown();
   void EngineStartup();
   void ExecutionStart();
   void ExecutionEnd();
   void Print(const QString& html, const QString& colorName);
   void PrintBr(const QString& html, const QString& colorName);

private slots:
   void OnPropertiesViewClosed();

protected slots:
   virtual void OnBeforeDeleteOwner(DataClass *dc);

private:

   QString ReadFileSourceCode();   
   void PerformScriptExecution(const QString& script);

   QString m_fileName, m_sourceCode;
   ScriptProcessorView* m_view;
   QJSEngine* m_scriptEngine;
   ScriptProcessor* m_sourceProcessor;
};

ScriptDocumentation *BuildScriptDocumentationScriptProcessor();

class ScriptProcessorSW : public BlockSW
{
   Q_OBJECT
public:
   ScriptProcessorSW(QJSEngine* se, ScriptProcessor* ide);

   Q_INVOKABLE void ClearContext();
   Q_INVOKABLE void LoadFile(const QString& fileName);
   Q_INVOKABLE void LoadSourceCode(const QString& sourceCode);

   Q_INVOKABLE void Print(const QString& html, const QString& colorName = "");
   Q_INVOKABLE void PrintBr(const QString& html = "", const QString& colorName = "");

   Q_INVOKABLE void Execute();
   Q_INVOKABLE bool AddOutput(double outputId, const QJSValue& instance);

   Q_INVOKABLE void ShowConfirmDialog(const QJSValue &input, const QString& message = "");
   Q_INVOKABLE void ShowDialog(const QString& message);
   Q_INVOKABLE QJSValue ShowInputDialog(const QString& message);
   Q_INVOKABLE QJSValue ShowYesNoDialog(const QString& message);
   Q_INVOKABLE QJSValue ShowOkCancelDialog(const QString& message);

signals:
   void Startup();
   void Shutdown();

private slots:
   void OnEngineStartup();
   void OnEngineShutdown();

private:
   ScriptProcessor* m_ide;
   QJSEngine* m_scriptEngine;

};

}
