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

#include <QIcon>
#include <QString>
#include <list>
#include "tools/TerbitDefs.h"

namespace terbit
{

class DataClass;
class DataClassType;
class ScriptDocumentation;

class FactoryTypeInfo
{
public:
   FactoryTypeInfo(const QString& typeName, DataClassKind kind, const QIcon& graphics32, const QString& displayName, const QString& description, ScriptDocumentation* scriptDocumentation)
      : m_typeName(typeName), m_displayName(displayName),m_description(description), m_kind(kind), m_graphics32(graphics32),m_scriptDocumentation(scriptDocumentation) {}

   const QString&  GetTypeName() const { return m_typeName; }
   DataClassKind GetKind() const { return m_kind; }
   const QIcon& GetGraphics32() const { return m_graphics32; }
   const QString& GetDisplayName() const {  return m_displayName; }
   const QString& GetDescription() const { return m_description; }
   ScriptDocumentation* GetScriptDocumentation () const { return m_scriptDocumentation; }
private:
   QString m_typeName, m_displayName, m_description,m_documentationJSON;
   DataClassKind m_kind;
   QIcon m_graphics32;
   ScriptDocumentation* m_scriptDocumentation;
};

class IDataClassFactory
{
public:
   IDataClassFactory() {}
   virtual ~IDataClassFactory() {}
   virtual const QString& GetName() const = 0;
   virtual const QString& GetProviderName() const = 0;
   virtual const QString& GetDescription() const = 0;
   virtual const std::vector<FactoryTypeInfo*>& GetTypeList() const = 0;
   virtual DataClass* CreateInstance(const QString& typeName) = 0;
};

}

// Reverse DNS company (com.terbit).Category/Purpose.Qt/c/cpp.modifier
// TODO: have build system update version?
#define IDataClassFactory_iid "com.terbittech.IDataClassFactory.Qt/0.2.0"
Q_DECLARE_INTERFACE(terbit::IDataClassFactory, IDataClassFactory_iid)
