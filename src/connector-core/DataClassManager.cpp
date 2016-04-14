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

#include <QUuid>
#include <QApplication>
#include "DataClassManager.h"
#include "tools/Tools.h"
#include "Plugin.h"
#include "IDataClassFactory.h"
#include "Workspace.h"
#include "LogDL.h"

namespace terbit
{

//0 is invalid key indicator
DataClassManager::DataClassManager() : m_nextKey(1), m_map()
{
}

DataClassManager::~DataClassManager()
{
   DeleteAll();

   //clear out the types, manager owns the instances
   for(DataClassTypeMap::iterator i = m_typeMap.begin(); i != m_typeMap.end(); i++)
   {
      delete i->second;
   }
   m_typeMap.clear();
}

void DataClassManager::DeleteKind(DataClassKind kind)
{
   //don't use iterator with delete inside loop
   //in case deletion causes removable from map or other data class objects to be removed
   //first build list and then remove . . .
   //call standard remove so it finds item first in case item has already been removed due to dependency
   DataClass* dc;
   std::vector<DataClassAutoId_t> l;
   l.reserve(m_map.size());
   for(DataClassMap::iterator i = m_map.begin(); i != m_map.end(); i++)
   {
      dc = i->second;
      if (dc->GetType()->GetKind() == kind)
      {
         l.push_back(i->first);
      }
   }

   for(std::vector<DataClassAutoId_t>::iterator j = l.begin(); j != l.end(); ++j)
   {
      Delete(*j);
   }
}

void DataClassManager::Add(DataClass* d, DataClass* owner, DataClassType* t, Workspace* ws, bool publicScope)
{
   m_objectMutex.lock();
   ++(t->m_instanceCount);
   DataClassAutoId_t key = GenerateKey();
   d->m_autoId = key;
   d->SetOwner(owner); //use function so we wire in events
   d->m_type = t;
   d->m_workspace = ws;
   d->m_publicScope = publicScope;
   d->m_uniqueId = QUuid::createUuid().toString();
   if (t->m_instanceCount == 1)
   {
      d->m_name = t->GetDisplayName();
   }
   else
   {
      d->m_name = QObject::tr("%1 %2").arg(t->GetDisplayName()).arg(t->m_instanceCount);
   }
   m_map.insert(std::pair<DataClassAutoId_t, DataClass*>(key, d));
   m_mapUniqueIds[d->m_uniqueId] = d;
   m_objectMutex.unlock();
}

DataClass* DataClassManager::Find(DataClassAutoId_t id)
{
   m_objectMutex.lock();
   DataClass *d = NULL;
   DataClassMap::iterator it = m_map.find(id);

   if(it != m_map.end())
   {
      d = it->second;
   }
   m_objectMutex.unlock();

   return d;
}

DataClass *DataClassManager::Find(const QString &uniqueId)
{
   m_objectMutex.lock();
   DataClass* dc = NULL;
   std::map<QString, DataClass*>::iterator it = m_mapUniqueIds.find(uniqueId);

   if(it != m_mapUniqueIds.end())
   {
      dc = it->second;
   }
   m_objectMutex.unlock();

   return dc;
}

bool DataClassManager::ReplaceUniqueId(const QString &oldUniqueId, const QString &newUniqueId)
{
   bool res = false;
   if (oldUniqueId == newUniqueId)
   {
      res = true;
   }
   else
   {
      m_objectMutex.lock();
      std::map<QString, DataClass*>::iterator it = m_mapUniqueIds.find(oldUniqueId);
      std::map<QString, DataClass*>::iterator dup = m_mapUniqueIds.find(newUniqueId);

      if (dup != m_mapUniqueIds.end())
      {
         LogError(g_log.general,QObject::tr("ReplaceUniqueId failed.  The new unique id is already used: %1").arg(newUniqueId));
      }
      else if(it != m_mapUniqueIds.end())
      {
         DataClass* dc = it->second;
         m_mapUniqueIds.erase(it);
         m_mapUniqueIds[newUniqueId] = dc;
         res = true;
      }
      else
      {
         LogError(g_log.general,QObject::tr("Couldn't find UniqueId to replace."));
      }
      m_objectMutex.unlock();
   }
   return res;
}

void DataClassManager::Delete(DataClassAutoId_t id)
{
   m_objectMutex.lock();
   DataClassMap::iterator it = m_map.find(id);

   if(it != m_map.end())
   {
      DataClass* d = it->second;      
      m_objectMutex.unlock();

      emit d->BeforeDeletion(d);
      d->ImpendingDeletion();

      QString uniqueId = d->GetUniqueId();
      delete d;

      //erase based on id in case the about delete causes other data class objects to be removed
      //removing dependencies will change the state of the map.
      m_objectMutex.lock();
      m_mapUniqueIds.erase(uniqueId);
      m_map.erase(id);
      m_objectMutex.unlock();
   }
   else
   {
      m_objectMutex.unlock();
   }
}

void DataClassManager::DeleteAll()
{
   //sometimes removing one item can remove other items
   //remove based on possible depedencies . . .
   //first remove displays
   //second processors
   //thrid devices
   //fourth sources

   DeleteKind(DATA_CLASS_KIND_DISPLAY);
   DeleteKind(DATA_CLASS_KIND_PROCESSOR);
   DeleteKind(DATA_CLASS_KIND_DEVICE);
   DeleteKind(DATA_CLASS_KIND_SOURCE);

}


DataClassAutoId_t DataClassManager::GenerateKey()
{
   return m_nextKey++;
}


void DataClassManager::AddTypes(Plugin* p, IDataClassFactory* f)
{
   //auto id is shared, so lock it just to make sure
   m_objectMutex.lock();
   //add all types from the factory for the plugin
   DataClassType* t;
   std::vector<FactoryTypeInfo*>::const_iterator i;
   for(i = f->GetTypeList().begin(); i != f->GetTypeList().end(); ++i)
   {
      const FactoryTypeInfo* ti = *i;
      //unique id's across types and instances
      QString fullTypeName = p->GetNamespace() + QString(".")  + ti->GetTypeName();
      t = new DataClassType(GenerateKey(), fullTypeName, ti->GetTypeName(), ti->GetKind(),ti->GetGraphics32(),DataClassType::FACTORY_TYPE_DATACLASS,(void *)f,p,ti->GetDisplayName(),ti->GetDescription(),ti->GetScriptDocumentation());

      m_typeMap.insert(std::pair<DataClassAutoId_t, DataClassType*>(t->GetId(), t));
   }
   m_objectMutex.unlock();
}

DataClass *DataClassManager::Create(const DataClassType* t)
{
   //No need to lock mutex here . . . just creates the raw instance

   DataClass* res = NULL;

   switch (t->GetFactoryType())
   {
      case DataClassType::FACTORY_TYPE_DATACLASS:
         {
            IDataClassFactory* idcf = (IDataClassFactory*)t->GetFactory();
            res = idcf->CreateInstance(t->GetTypeName());
         }
         break;
   }

   //make sure it's under the GUI thread
   if (res)
   {
      res->moveToThread(QCoreApplication::instance()->thread());
   }
   return res;
}

DataClassType* DataClassManager::FindType(DataClassAutoId_t id)
{
   //assume types not added from a thread ...

   DataClassType *d = NULL;
   DataClassTypeMap::iterator it = m_typeMap.find(id);

   if(it != m_typeMap.end())
   {
      d = it->second;
   }

   return d;
}

DataClassType *DataClassManager::FindType(const QString &fullTypeName)
{
   //assume types not added from a thread ...

   for(auto& i : m_typeMap)
   {
      DataClassType* t = i.second;
      if (t->GetFullTypeName() == fullTypeName)
      {
         return t;
      }
   }
   return NULL;
}

void DataClassManager::BuildDependencyList(std::list<DataClass*>& dependencies)
{
   //default dependency order based on kind
   //data class can always explicitly specify their dependencies of any kind and that will be honored

   m_objectMutex.lock();
   BuildDependencyList(dependencies, DATA_CLASS_KIND_DEVICE);
   BuildDependencyList(dependencies, DATA_CLASS_KIND_PROCESSOR);
   BuildDependencyList(dependencies, DATA_CLASS_KIND_SOURCE);
   BuildDependencyList(dependencies, DATA_CLASS_KIND_DISPLAY);
   m_objectMutex.unlock();
}

void DataClassManager::BuildDependencyList(std::list<DataClass *> &dependencies, DataClassKind kind)
{
   for(auto& i : m_map)
   {
      DataClass* dc = i.second;

      if (dc->GetType()->GetKind() == kind)
      {
         std::list<DataClass*> current;

         //see if something already depends on current dc
         std::list<DataClass*>::iterator di;
         for(di = dependencies.begin(); di != dependencies.end(); ++di)
         {
            if (*di == dc)
            {
               break;
            }
         }
         if (di == dependencies.end())
         {
            current.push_back(dc); // new to the fray
         }
         else
         {
            //splice off current node and everything that depends on it
            current.splice(current.begin(), dependencies, di, dependencies.end());
         }

         std::list<DataClass*> dependsOn;
         dc->GetDirectDependencies(dependsOn);

         //add depends on to the list if DNE
         for(DataClass* d : dependsOn)
         {
            bool found = false;
            for(DataClass* e : dependencies)
            {
               if (e == d)
               {
                  found = true;
                  break;
               }
            }
            if (!found)
            {
               dependencies.push_back(d);
            }
         }

         //now splice in the current and list of everything that depends on it
         dependencies.splice(dependencies.end(), current);
      }
   }
}

}
