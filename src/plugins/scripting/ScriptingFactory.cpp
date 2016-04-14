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
#include "ScriptingFactory.h"
#include "ScriptProcessor.h"
#include "ScriptDisplay.h"

//resource init must be outside namespace and needed when used in a library
void TerbitScriptingResourceInitialize()
{
   Q_INIT_RESOURCE(scripting);
}

namespace terbit
{

ScriptingFactory::ScriptingFactory()
{
   TerbitScriptingResourceInitialize();

   m_provider = "Terbit";
   m_name     = "Scripting";
   m_description = QObject::tr("Application scripting and data processing.");

   QString display, description;

   display = QObject::tr("Script");
   description = QObject::tr("A script-based processor.");
   m_typeList.push_back(new FactoryTypeInfo(SCRIPTPROCESSOR_TYPENAME, DATA_CLASS_KIND_PROCESSOR, QIcon(":/images/32x32/script_code.png"),display,description,BuildScriptDocumentationScriptProcessor()));

   display = QObject::tr("HTML Display");
   description = QObject::tr("A HTML display.");
   m_typeList.push_back(new FactoryTypeInfo(SCRIPTDISPLAY_TYPENAME, DATA_CLASS_KIND_DISPLAY, QIcon(":/images/32x32/script_code.png"),display,description,BuildScriptDocumentationScriptDisplay()));

}

ScriptingFactory::~ScriptingFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &ScriptingFactory::GetProviderName() const
{
   return m_provider;
}

const QString &ScriptingFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& ScriptingFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* ScriptingFactory::CreateInstance(const QString& typeName)
{
   if (typeName == SCRIPTPROCESSOR_TYPENAME)
   {
      return new ScriptProcessor();
   }
   else if (typeName == SCRIPTDISPLAY_TYPENAME)
   {
      return new ScriptDisplay();
   }
   else
   {
      return NULL;
   }
}
}
