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
#include <QApplication>
#include <QQmlContext>
#include <QDateTime>
#include "ScriptingUtils.h"
#include "ScriptProcessor.h"
#include "TerbitSW.h"
#include <tools/TerbitDefs.h>
#include <connector-core/Workspace.h>
#include "plugins/scripting/TimerLibSW.h"
#include "plugins/scripting/FileIOLibSW.h"

namespace terbit
{

void ScriptingGlobalStartup()
{
}

void ScriptingGlobalShutdown()
{

}

void ScriptingInitEngine(QJSEngine *engine, ScriptProcessor *ide, Workspace *w)
{   
   //terbit
   auto terbit = new TerbitSW(engine, ide,w);
   engine->globalObject().setProperty("terbit", engine->newQObject(terbit));
}


QString QJSValueToString(const QJSValue &value)
{
   QString res;
   if (value.isString())
   {
      res = value.toString();
   }
   else if (value.isNumber())
   {
      res = DoubleToStringComplete(value.toNumber());
   }
   else if (value.isBool())
   {
      res = (value.toBool() ? "true" : "false");
   }
   else if (value.isDate())
   {
      res = value.toDateTime().toString(Qt::ISODate);
   }

   return res;
}

ScriptDocumentation * BuildScriptEnvironmentDocumentation()
{
   ScriptDocumentation* res = new ScriptDocumentation();

   res->SetName(QObject::tr("Environment"));
   res->SetSummary(QObject::tr("Terbit scripting environment extensions."));

   res->AddSubDocumentation(BuildScriptDocumentationTerbit());
   res->AddSubDocumentation(BuildScriptDocumentationWorkspace());
   res->AddSubDocumentation(BuildScriptDocumentationFileIOLibSW());
   res->AddSubDocumentation(BuildScriptDocumentationTimerLibSW());

   return res;
}



}
