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
#include <map>
#include <tools/TerbitDefs.h>
#include "plugins/scripting/ScriptBuilder.h"
#include "tools/Script.h"

namespace terbit
{

class DataClass;

class BlockIOContainer : public QObject
{
   Q_OBJECT
public:
   typedef std::multimap<BlockIOCategory_t, DataClass*>::iterator iterator;
   iterator begin() { return m_map.begin(); }
   iterator end() { return m_map.end(); }

   void Add(BlockIOCategory_t category, DataClass* dc);
   bool RemoveAll(BlockIOCategory_t category);
   bool RemoveFirst(BlockIOCategory_t category);
   DataClass* FindFirst(BlockIOCategory_t category);
   iterator Find(BlockIOCategory_t category) { return m_map.find(category); }

private:
   std::multimap<BlockIOCategory_t, DataClass*> m_map;
};

ScriptDocumentation* BuildScriptDocumentationBlockIOContainer();

class BlockIOContainerSW : public QObject
{
   Q_OBJECT
public:
   BlockIOContainerSW(QJSEngine* se, BlockIOContainer* io);
   virtual ~BlockIOContainerSW() {}

   Q_INVOKABLE QJSValue FindFirst(const QJSValue& category);
private:
   QJSEngine* m_scriptEngine;
   BlockIOContainer* m_io;
};

}
