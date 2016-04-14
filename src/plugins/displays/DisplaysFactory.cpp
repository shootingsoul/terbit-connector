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

#include "DisplaysFactory.h"
#include "DataSetValues.h"

//resource init must be outside namespace and needed when used in a library
void TerbitDisplaysResourceInitialize()
{
   Q_INIT_RESOURCE(displays);
}

namespace terbit
{

DisplaysFactory::DisplaysFactory()
{
   TerbitDisplaysResourceInitialize();

   m_provider = "Terbit";
   m_name     = "Displays";
   m_description = QObject::tr("General Connector displays.");

   QString display, description;

   display = QObject::tr("Data Set Values");
   description = QObject::tr("View the values in a data set.  Supports hexadecimal and octal displays.");
   m_typeList.push_back(new FactoryTypeInfo(QString(DATASETVALUES_TYPENAME), DATA_CLASS_KIND_DISPLAY, QIcon(":/images/32x32/table.png"),display,description,BuildScriptDocumentationDataSetValuesDisplay()));
}

DisplaysFactory::~DisplaysFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &DisplaysFactory::GetProviderName() const
{
   return m_provider;
}

const QString &DisplaysFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& DisplaysFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* DisplaysFactory::CreateInstance(const QString& typeName)
{
   if (typeName == DATASETVALUES_TYPENAME)
   {
      return new DataSetValues();
   }
   else
   {
      return NULL;
   }
}
}
