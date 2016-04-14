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
#include <vector>
#include <QString>
#include <QIcon>
#include "tools/TerbitDefs.h"
#include "ScriptDocumentation.h"

namespace terbit
{

class Plugin;


class DataClassType
{
   friend class DataClassManager;
public:

   enum FactoryType
   {
      FACTORY_TYPE_DATACLASS = 0
   };

   DataClassType(DataClassAutoId_t id, const QString& fullTypeName, const QString& typeName, DataClassKind kind, const QIcon& graphics32, FactoryType factoryType, void* factory, Plugin* plugin, const QString& displayName, const QString& description, ScriptDocumentation* documentation);
   ~DataClassType();

   const QString& GetFullTypeName() const { return m_fullTypeName; }
   const QString& GetTypeName() const { return m_typeName; }
   DataClassKind GetKind() const { return m_kind; }
   const QIcon& GetGraphics32() const { return m_graphics32; }
   uint32_t GetLogCategory() const { return m_logCategory; }

   FactoryType GetFactoryType()  const { return m_factoryType; }
   void* GetFactory() const { return m_factory; }

   const Plugin* GetPlugin() const { return m_plugin; }
   DataClassAutoId_t GetId(void) const { return m_id; }

   const QString& GetDisplayName() const { return m_displayName; }
   const QString& GetDescription() const { return m_description; }

   size_t GetInstanceCount() const { return m_instanceCount; }

   QObject* CreateScriptWrapper(QJSEngine* se) const;

   const ScriptDocumentation* GetScriptDocumentation() const { return m_scriptDocumentation; }

 private:
   DataClassType(const DataClassType& o); //disable copy ctor

   DataClassAutoId_t m_id;
   QString m_fullTypeName;
   QString m_typeName;
   DataClassKind m_kind;
   QIcon m_graphics32;
   FactoryType m_factoryType;
   void* m_factory;
   Plugin* m_plugin;
   QString m_displayName;
   QString m_description;
   uint32_t m_logCategory;
   size_t m_instanceCount;
   ScriptDocumentation* m_scriptDocumentation;
};

typedef std::map<DataClassAutoId_t, DataClassType*> DataClassTypeMap;

class DataClassTypeSW : public QObject
{
   Q_OBJECT
public:
   DataClassTypeSW(QJSEngine* se, const DataClassType* type);
   virtual ~DataClassTypeSW() {}

   Q_INVOKABLE QString GetFullTypeName();

protected:
   QJSEngine* m_scriptEngine;
   const DataClassType* m_type;
};
}
