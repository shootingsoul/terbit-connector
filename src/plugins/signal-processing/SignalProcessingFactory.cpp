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
#include "SignalProcessingFactory.h"
#include "SigAnalysisProcessor.h"
#include "SigAnalysisProcSW.h"

//resource init must be outside namespace and needed when used in a library
void TerbitSignalProcessingResourceInitialize()
{
   Q_INIT_RESOURCE(signalprocessing);
}

namespace terbit
{

SignalProcessingFactory::SignalProcessingFactory()
{
   TerbitSignalProcessingResourceInitialize();

   m_provider = "Terbit";
   m_name     = "SignalProcessing";
   m_description = QObject::tr("Signal processing functions");

   QString display, description;

   display = QObject::tr("Signal Analysis");
   description = QObject::tr("FFT and performance metrics, with options.");
   m_typeList.push_back(new FactoryTypeInfo(SIG_ANALYSIS_PROCESSOR_TYPENAME, DATA_CLASS_KIND_PROCESSOR, QIcon(":/images/32x32/function.png"), display, description,BuildScriptDocumentationSigAnalysisProc()));
}

SignalProcessingFactory::~SignalProcessingFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &SignalProcessingFactory::GetProviderName() const
{
   return m_provider;
}

const QString &SignalProcessingFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& SignalProcessingFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* SignalProcessingFactory::CreateInstance(const QString& typeName)
{
   if (typeName == SIG_ANALYSIS_PROCESSOR_TYPENAME)
   {
      return new SigAnalysisProcessor();
   }
   else
   {
      return NULL;
   }
}

}
