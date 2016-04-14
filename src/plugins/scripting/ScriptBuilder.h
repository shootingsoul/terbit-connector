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

#include <QString>
#include <vector>
#include "tools/TerbitDefs.h"

namespace terbit
{

class DataClass;
class DataClassType;
class Workspace;
class Block;
class DataSet;
class Scriptlet;
class ScriptDocumentation;

class ScriptBuilder
{
public:
   ScriptBuilder();

   static const QString VARIABLE_WORKSPACE;
   static const QString VARIABLE_TERBIT;

   void add(const QString& line);
   QString ToScriptString();

   enum TypeScriptlets
   {
      TYPE_FULL_NAME = 0,
      TYPE_CREATE = 1,
      TYPE_DOCUMENTATION = 2
   };

   enum InstanceScriptlets
   {
      INSTANCE_UNIQUE_ID = 0,
      INSTANCE_FIND = 1,
      INSTANCE_RESTORE = 2
   };

   static QString GenerateScript(TerbitDataType td);
   static QString GenerateScript(TypeScriptlets scriptlet, const DataClassType* type);
   static QString GenerateScript(InstanceScriptlets scriptlet, DataClass* dc);
   static QString GenerateFullRestoreScript(Workspace* ws);
   static QString GenerateFullDocumentationScript(Workspace* ws, ScriptDocumentation* environmentDocumentation);
   static QString GenerateDocumentation(Scriptlet* scriptlet);
   static QString GenerateDocumentation(ScriptDocumentation* doc);
   static QString GeneratePrintScript();
   static QString GeneratePrintBrScript();
private:
   static QString GenerateFullNameScript(const DataClassType* type);
   static QString GenerateCreateScript(const DataClassType* type);
   static QString GenerateDocumentationScript(const DataClassType* type);

   static QString GenerateUniqueIdScript(DataClass* dc);
   static QString GenerateFindScript(DataClass* dc);
   static QString GenerateRestoreScript(DataClass* dc);

   static void BuildRestoreScript(ScriptBuilder& script, Workspace* ws);
   static void BuildRestoreInstance(ScriptBuilder &script, const QString &variableName, DataClass *dc);
   static void BuildRestoreBlock(ScriptBuilder &script, const QString &variableName, Block *dc);
   static void BuildRestoreRemoteDataSet(ScriptBuilder &script, const QString &variableName, DataSet *buf);
   static void BuildRestoreNameUniqueId(ScriptBuilder& script, const QString& variableName, DataClass* dc);

   QString m_script;
};


}
