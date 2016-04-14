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

#include "CoreFactory.h"
#include "DataSet.h"
#include "DataSet.h"

namespace terbit
{

CoreFactory::CoreFactory()
{
   m_provider = "Terbit";
   m_name     = "Core";
   m_description = QObject::tr("The core Connector library.");

   QString display, description;

   display = QObject::tr("Data Set");
   description = QObject::tr("Represents a typed physical memory buffer with a stride.");
   m_typeList.push_back(new FactoryTypeInfo(QString(DATASET_TYPENAME), DATA_CLASS_KIND_SOURCE, QIcon(":/images/32x32/database_green.png"),display,description,BuildScriptDocumentationDataSet()));

}

CoreFactory::~CoreFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &CoreFactory::GetProviderName() const
{
   return m_provider;
}

const QString &CoreFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& CoreFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* CoreFactory::CreateInstance(const QString& typeName)
{
   if (typeName == DATASET_TYPENAME)
   {
      return new DataSet();
   }
   else
   {
      return NULL;
   }
}

}
