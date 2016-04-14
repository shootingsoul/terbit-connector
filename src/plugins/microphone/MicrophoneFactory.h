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
#include <QtPlugin>
#include "Microphone_global.h"
#include "connector-core/IDataClassFactory.h"

namespace terbit
{

class MicrophoneFactory : public QObject, public terbit::IDataClassFactory
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID IDataClassFactory_iid FILE "MicrophoneFactory.json")
   Q_INTERFACES(terbit::IDataClassFactory)

public:
   MicrophoneFactory();
   ~MicrophoneFactory();

   const QString& GetName() const {return m_name;}
   virtual const QString& GetProviderName() const;
   virtual const QString& GetDescription() const;
   virtual const std::vector<FactoryTypeInfo*>& GetTypeList() const;
   virtual DataClass* CreateInstance(const QString& typeName);
private:
   std::vector<FactoryTypeInfo*> m_typeList;
   QString m_provider;
   QString m_name;
   QString m_description;
};

}
