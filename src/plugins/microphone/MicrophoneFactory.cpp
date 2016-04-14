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
#include "MicrophoneFactory.h"
#include "MicrophoneDevice.h"

//resource init must be outside namespace and needed when used in a library
void TerbitMicrophoneResourceInitialize()
{
   Q_INIT_RESOURCE(microphone);
}

namespace terbit
{

MicrophoneFactory::MicrophoneFactory()
{
   TerbitMicrophoneResourceInitialize();

   m_provider = "Terbit";
   m_name     = "Microphone";
   m_description = QObject::tr("Access the built-in microphone device(s).");

   QString display, description;

   display = QObject::tr("Microphone");
   description = QObject::tr("A built-in microphone device.");
   m_typeList.push_back(new FactoryTypeInfo(MICROPHONEDEVICE_TYPENAME, DATA_CLASS_KIND_DEVICE, QIcon(":/images/32x32/network_adapter.png"),display,description,BuildScriptDocumentationMicrophone()));
}

MicrophoneFactory::~MicrophoneFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &MicrophoneFactory::GetProviderName() const
{
   return m_provider;
}

const QString &MicrophoneFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& MicrophoneFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* MicrophoneFactory::CreateInstance(const QString& typeName)
{
   if (typeName == MICROPHONEDEVICE_TYPENAME)
   {
      return new MicrophoneDevice();
   }
   else
   {
      return NULL;
   }
}
}
