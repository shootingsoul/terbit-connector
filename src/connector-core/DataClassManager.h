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

#include <QMutex>

#include "DataClass.h"
#include "DataClassType.h"

namespace terbit
{

class Plugin;
class IDataClassFactory;

/*!
 * @brief The DataClassManager class owns the data class objects . . . sets ids, deletes them
 */
class DataClassManager
{
   friend class Workspace;
public:
   DataClassManager();
   ~DataClassManager();

   DataClass* Find(DataClassAutoId_t id);
   DataClass* Find(const QString& uniqueId);

   bool ReplaceUniqueId(const QString& oldUniqueId, const QString& newUniqueId);

   DataClassType* FindType(DataClassAutoId_t id);
   DataClassType* FindType(const QString& fullTypeName);

   const DataClassTypeMap& GetTypes() { return m_typeMap; }
   const DataClassMap& GetInstances() { return m_map; }

   void BuildDependencyList(std::list<DataClass*>& dependencies);

private:
   DataClassManager(const DataClassManager& o); //disable copy ctor

   //type management only accessable from workspace
   void AddTypes(Plugin* p, IDataClassFactory* f);

   //instance create/add/remove only accessable from Workspace
   DataClass* Create(const DataClassType* t);
   void Add(DataClass* d, DataClass* owner, DataClassType* t, Workspace* ws, bool publicScope);
   void Delete(DataClassAutoId_t id);
   void DeleteAll();

   void BuildDependencyList(std::list<DataClass*>& dependencies, DataClassKind kind);

   void DeleteKind(DataClassKind kind);

   DataClassAutoId_t GenerateKey();

   QMutex m_objectMutex;
   DataClassAutoId_t m_nextKey;
   DataClassMap m_map;
   std::map<QString, DataClass*> m_mapUniqueIds;
   DataClassTypeMap m_typeMap;
};

}
