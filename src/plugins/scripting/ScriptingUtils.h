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

#include <QObject>
#include <QJSEngine>
#include <QJSValue>
#include <vector>
#include <connector-core/ScriptDocumentation.h>

namespace terbit
{
   class ScriptProcessor;
   class Workspace;

   void ScriptingGlobalStartup();
   void ScriptingGlobalShutdown();

   void ScriptingInitEngine(QJSEngine *engine, ScriptProcessor *ide, Workspace *w);

   QString QJSValueToString(const QJSValue& value);

   ScriptDocumentation* BuildScriptEnvironmentDocumentation();

}
