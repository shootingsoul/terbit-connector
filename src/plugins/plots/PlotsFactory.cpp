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
#include "PlotsFactory.h"
#include "XYPlot.h"

//resource init must be outside namespace and needed when used in a library
void TerbitPlotsResourceInitialize()
{
   Q_INIT_RESOURCE(plots);
}

namespace terbit
{

PlotsFactory::PlotsFactory()
{
   TerbitPlotsResourceInitialize();

   m_provider = "Terbit";
   m_name     = "Plots";
   m_description = QObject::tr("Connector plotting library.");

   QString display, description;

   display = QObject::tr("Line Plot");
   description = QObject::tr("Line plot with support for multiple series and zooming.");
   m_typeList.push_back(new FactoryTypeInfo(QString(XYPLOT_TYPENAME), DATA_CLASS_KIND_DISPLAY, QIcon(":/images/32x32/linechart.png"),display,description,BuildScriptDocumentationXYPlot()));

}

PlotsFactory::~PlotsFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &PlotsFactory::GetProviderName() const
{
   return m_provider;
}

const std::vector<FactoryTypeInfo*>& PlotsFactory::GetTypeList() const
{
   return m_typeList;
}

const QString &PlotsFactory::GetDescription() const
{
   return m_description;
}

DataClass* PlotsFactory::CreateInstance(const QString& typeName)
{
   if (typeName == XYPLOT_TYPENAME)
   {
      return new XYPlot();
   }
   else
   {
      return NULL;
   }
}
}
