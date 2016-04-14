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
#include "TerbitSW.h"
#include "FrequencyMetricsLibSW.h"
#include "FileIOLibSW.h"
#include "TimerLibSW.h"
#include "ScriptProcessor.h"
#include "ScriptingUtils.h"
#include <connector-core/Workspace.h>
#include <tools/Tools.h>

namespace terbit
{

TerbitSW::TerbitSW(QJSEngine *engine, ScriptProcessor *ide, Workspace *w)
   : m_scriptEngine(engine), m_ide(ide), m_workspace(w)
{
   m_ws = m_scriptEngine->newQObject(new WorkspaceSW(engine,w, ide->GetSourceProcessor()));
}

#undef LogError
#undef LogWarning
#undef LogInfo

void TerbitSW::LogError(const QJSValue &msg)
{
   LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), QJSValueToString(msg));
}

void TerbitSW::LogWarning(const QJSValue &msg)
{
   LogWarning2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), QJSValueToString(msg));
}

void TerbitSW::LogInfo(const QJSValue &msg)
{
   LogInfo2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), QJSValueToString(msg));
}

void TerbitSW::Print(const QJSValue &html, const QString& colorName)
{
   emit m_ide->Print(QJSValueToString(html), colorName);
}

void TerbitSW::PrintBr(const QJSValue &html, const QString& colorName)
{
   emit m_ide->PrintBr(QJSValueToString(html), colorName);
}

QJSValue TerbitSW::GetCurrentScript()
{
   return m_scriptEngine->newQObject(new ScriptProcessorSW(m_scriptEngine,m_ide));
}

QJSValue TerbitSW:: CreateFileIO()
{
   return m_scriptEngine->newQObject(new FileIOLibSW(m_workspace, m_ide, m_scriptEngine));
}

QJSValue TerbitSW:: CreateTimer()
{
   return m_scriptEngine->newQObject(new TimerLibSW(m_workspace, m_ide, m_scriptEngine));
}

ScriptDocumentation *BuildScriptDocumentationTerbit()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->SetName("terbit");
   d->SetSummary(TerbitSW::tr("Access to application and utilities."));
   d->AddScriptlet(new Scriptlet("workspace","terbit.workspace",TerbitSW::tr("A variable to access the workspace for the application.  See the Workspace documentation for more information.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("Current Script"),"terbit.GetCurrentScript();",TerbitSW::tr("Reference current script processor.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("LogInfo"),"LogInfo(msg);",TerbitSW::tr("Post a message to the application log queue.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("LogWarning"),"LogWarning(msg);",TerbitSW::tr("Post a message to the application log queue.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("LogError"),"LogError(msg);",TerbitSW::tr("Post a message to the application log queue.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("Print"),ScriptBuilder::GeneratePrintScript(),TerbitSW::tr("Displays the message specified by the string msg in the color specified by the string color.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("PrintBr"),ScriptBuilder::GeneratePrintBrScript(),TerbitSW::tr("Displays the message specified by the string msg in the color specified by the string color.  The br tag is automatically added to the end.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("CreateFileIO"),"CreateFileIO();",TerbitSW::tr("Create an instance of a FileIOLibSW object, which allows reading and writing of files.")));
   d->AddScriptlet(new Scriptlet(TerbitSW::tr("CreateTimer"),"CreateTimer();",TerbitSW::tr("Create an instance of a TimerLibSW object, which provides QTimer functionality.")));


   ScriptDocumentation* dt = new ScriptDocumentation();
   dt->SetName(TerbitSW::tr("Data Types"));
   dt->SetSummary(TerbitSW::tr("Terbit data types"));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("UINT8"),ScriptBuilder::GenerateScript(TERBIT_UINT8),TerbitSW::tr("unsigned 8-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("UINT16"),ScriptBuilder::GenerateScript(TERBIT_UINT16),TerbitSW::tr("unsigned 16-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("UINT32"),ScriptBuilder::GenerateScript(TERBIT_UINT32),TerbitSW::tr("unsigned 32-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("UINT64"),ScriptBuilder::GenerateScript(TERBIT_UINT64),TerbitSW::tr("unsigned 64-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("INT8"),ScriptBuilder::GenerateScript(TERBIT_INT8),TerbitSW::tr("signed 8-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("INT16"),ScriptBuilder::GenerateScript(TERBIT_INT16),TerbitSW::tr("signed 16-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("INT32"),ScriptBuilder::GenerateScript(TERBIT_INT32),TerbitSW::tr("signed 32-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("INT64"),ScriptBuilder::GenerateScript(TERBIT_INT64),TerbitSW::tr("signed 64-bit integer")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("FLOAT"),ScriptBuilder::GenerateScript(TERBIT_FLOAT),TerbitSW::tr("single-precision floating point")));
   dt->AddScriptlet(new Scriptlet(TerbitSW::tr("DOUBLE"),ScriptBuilder::GenerateScript(TERBIT_DOUBLE),TerbitSW::tr("double-precision floating point")));

   d->AddSubDocumentation(dt);

   return d;
}



}
