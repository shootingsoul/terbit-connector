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

#include <QJSEngine>
#include "DataClassType.h"
#include <tools/Tools.h>

namespace terbit
{

DataClassType::DataClassType(DataClassAutoId_t id, const QString &fullTypeName, const QString &typeName, DataClassKind kind, const QIcon& graphics32, DataClassType::FactoryType factoryType, void *factory, Plugin *plugin, const QString& displayName, const QString& description, ScriptDocumentation* scriptDocumentation)
 : m_id(id), m_fullTypeName(fullTypeName), m_typeName(typeName), m_kind(kind), m_graphics32(graphics32), m_factoryType(factoryType), m_factory(factory), m_plugin(plugin), m_displayName(displayName), m_description(description), m_instanceCount(0), m_scriptDocumentation(scriptDocumentation)
{
   m_logCategory = g_logManager.RegisterCategory(m_fullTypeName);

   if (m_scriptDocumentation == NULL)
   {
      m_scriptDocumentation = new ScriptDocumentation();
   }
}

DataClassType::~DataClassType()
{
   delete m_scriptDocumentation;
}

QObject* DataClassType::CreateScriptWrapper(QJSEngine *se) const
{
   return new DataClassTypeSW(se,this);
}

DataClassTypeSW::DataClassTypeSW(QJSEngine *se, const DataClassType *type) : m_scriptEngine(se), m_type(type)
{

}

QString DataClassTypeSW::GetFullTypeName()
{
   return m_type->GetFullTypeName();
}

}
