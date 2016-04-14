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

#include "FileDeviceFactory.h"
#include "FileDevice.h"

//resource init  must be outside namespace and needed when used in a library
void TerbitFileDvcResourceInitialize()
{
   //Q_INIT_RESOURCE(FileDvc_DP);
}

namespace terbit
{

FileDeviceFactory::FileDeviceFactory()
{
   TerbitFileDvcResourceInitialize();

   m_provider = "Terbit";
   m_name     = "FilePlayer";
   m_description = QObject::tr("Access data in a binary file.");

   QString display, description;

   display = QObject::tr("File Player");
   description = QObject::tr("A basic file playback device.");
   m_typeList.push_back(new FactoryTypeInfo(FILE_DEVICE_TYPENAME, DATA_CLASS_KIND_DEVICE, QIcon(":/images/32x32/network_adapter.png"),display,description,BuildScriptDocumentationFileDevice()));

}

FileDeviceFactory::~FileDeviceFactory()
{
   for(std::vector<FactoryTypeInfo*>::iterator i = m_typeList.begin(); i != m_typeList.end(); ++i)
   {
      delete *i;
   }
}

const QString &FileDeviceFactory::GetProviderName() const
{
   return m_provider;
}

const QString &FileDeviceFactory::GetDescription() const
{
   return m_description;
}

const std::vector<FactoryTypeInfo*>& FileDeviceFactory::GetTypeList() const
{
   return m_typeList;
}

DataClass* FileDeviceFactory::CreateInstance(const QString& typeName)
{
   if (typeName == FILE_DEVICE_TYPENAME)
   {
      return new(std::nothrow) FileDevice();
   }
   else
   {
      return NULL;
   }
}

}
