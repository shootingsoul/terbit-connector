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

#include <QJSEngine>
#include <QObject>
#include <tools/TerbitDefs.h>

namespace terbit
{

class ScriptProcessor;
class Workspace;
class ScriptDocumentation;

ScriptDocumentation* BuildScriptDocumentationTerbit();

class TerbitSW : public QObject
{
   Q_OBJECT

public:
   TerbitSW(QJSEngine *engine, ScriptProcessor *ide, Workspace *w);

#undef LogInfo
#undef LogWarning
#undef LogError
   Q_INVOKABLE void LogInfo(const QJSValue& msg);
   Q_INVOKABLE void LogWarning(const QJSValue& msg);
   Q_INVOKABLE void LogError(const QJSValue& msg);
   Q_INVOKABLE void Print(const QJSValue& html, const QString& colorName = "");
   Q_INVOKABLE void PrintBr(const QJSValue& html = "", const QString& colorName = "");

   Q_PROPERTY(QJSValue workspace READ GetWorkspace)
   QJSValue GetWorkspace() { return m_ws; }

   Q_PROPERTY(QJSValue UINT8 READ GetUINT8)
   QJSValue GetUINT8() { return TERBIT_UINT8; }

   Q_PROPERTY(QJSValue UINT16 READ GetUINT16)
   QJSValue GetUINT16() { return TERBIT_UINT16; }

   Q_PROPERTY(QJSValue UINT32 READ GetUINT32)
   QJSValue GetUINT32() { return TERBIT_UINT32; }

   Q_PROPERTY(QJSValue UINT64 READ GetUINT64)
   QJSValue GetUINT64() { return TERBIT_UINT64; }

   Q_PROPERTY(QJSValue INT8 READ GetINT8)
   QJSValue GetINT8() { return TERBIT_INT8; }

   Q_PROPERTY(QJSValue INT16 READ GetINT16)
   QJSValue GetINT16() { return TERBIT_INT16; }

   Q_PROPERTY(QJSValue INT32 READ GetINT32)
   QJSValue GetINT32() { return TERBIT_INT32; }

   Q_PROPERTY(QJSValue INT64 READ GetINT64)
   QJSValue GetINT64() { return TERBIT_INT64; }

   Q_PROPERTY(QJSValue FLOAT READ GetFLOAT)
   QJSValue GetFLOAT() { return TERBIT_FLOAT; }

   Q_PROPERTY(QJSValue DOUBLE READ GetDOUBLE)
   QJSValue GetDOUBLE() { return TERBIT_DOUBLE; }

   Q_INVOKABLE QJSValue GetCurrentScript();
   Q_INVOKABLE QJSValue CreateFileIO();   
   Q_INVOKABLE QJSValue CreateTimer();

private:
   QJSEngine* m_scriptEngine;
   ScriptProcessor *m_ide;
   Workspace* m_workspace;
   QJSValue m_ws;
};


}
