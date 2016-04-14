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

#include "BlockIOContainer.h"
#include "LogDL.h"
#include "ScriptDocumentation.h"
#include "DataClass.h"

namespace terbit
{

void BlockIOContainer::Add(BlockIOCategory_t category, DataClass *dc)
{
   m_map.insert(std::pair<BlockIOCategory_t,DataClass*>(category,dc));
}

bool BlockIOContainer::RemoveAll(BlockIOCategory_t category)
{
  return (m_map.erase(category) > 0);
}

bool BlockIOContainer::RemoveFirst(BlockIOCategory_t category)
{
  iterator i = m_map.find(category);
  if (i != m_map.end())
  {
     m_map.erase(i);
     return true;
  }
  else
  {
     return false;
  }
}

DataClass *BlockIOContainer::FindFirst(BlockIOCategory_t category)
{
   iterator i = m_map.find(category);
   if (i != m_map.end())
   {
      return i->second;
   }
   else
   {
      return NULL;
   }
}

ScriptDocumentation* BuildScriptDocumentationBlockIOContainer()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->SetName(QObject::tr("BlockIOContainer"));
   d->SetSummary(QObject::tr("Functions to work the list of input or output data classes for a block."));

   d->AddScriptlet(new Scriptlet(QObject::tr("FindFirst"), "FindFirst(category);",QObject::tr("Return a reference to the first data class for the numeric category.  The category is a constant.")));

   return d;
}

BlockIOContainerSW::BlockIOContainerSW(QJSEngine *se, BlockIOContainer *io) : m_scriptEngine(se), m_io(io)
{
}

QJSValue BlockIOContainerSW::FindFirst(const QJSValue& category)
{
   QJSValue res;

   if (category.isNumber())
   {
      BlockIOCategory_t cat = category.toUInt();
      DataClass* output = m_io->FindFirst(cat);
      if (output)
      {
         res = m_scriptEngine->newQObject(output->CreateScriptWrapper(m_scriptEngine));
      }
      else
      {
         LogError(g_log.general,tr("BlockIOContainer FindFirst output for category not found: %2").arg(cat));
      }
   }
   else
   {
      LogError(g_log.general,tr("BlockIOContainer FindFirst expects a numeric category."));
   }

   return res;
}


}
