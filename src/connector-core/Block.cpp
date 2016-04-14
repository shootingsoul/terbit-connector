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

#include "Block.h"
#include "LogDL.h"
#include "Workspace.h"

namespace terbit
{

Block::Block() : DataClass()
{

}

Block::~Block()
{

}

bool Block::IsBlock() const
{
   return true;
}

void Block::ApplyInput(DataClassAutoId_t id)
{
   DataClass* dc = m_workspace->FindInstance(id);
   if (dc)
   {
      ApplyInputDataClass(dc);
   }
   else
   {
      LogError2(GetType()->GetLogCategory(), GetName(),tr("Failed to add input.  Data class not found."));
   }
}
void Block::ApplyInputDataClass(DataClass* dc)
{
  m_inputs.Add(BLOCK_IO_CATEGORY_GENERAL, dc);
}

bool Block::AddOutput(BlockIOCategory_t category, DataClass *output)
{
   bool res = false;

   if (output)
   {      
      DataClass* dup = m_outputs.FindFirst(category);
      if (!dup)
      {
         m_outputs.Add(category,output);
         output->m_outputCategory = category;
         emit OutputAdded(this, output);
         res = true;
      }
      else
      {
         LogError2(GetType()->GetLogCategory(), GetName(),tr("Failed to add output.  Duplicate output for category: %1").arg(category));
      }
   }

   return res;
}

bool Block::RemoveOutput(BlockIOCategory_t category)
{
   return m_outputs.RemoveFirst(category);
}


ScriptDocumentation *BuildScriptDocumentationBlock()
{
   ScriptDocumentation* d = BuildScriptDocumentationDataClass();

   d->AddScriptlet(new Scriptlet(QObject::tr("GetInputs"), "GetInputs();",QObject::tr("Return a reference to the outputs.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetOutputs"), "GetOutputs();",QObject::tr("Return to the inputs.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Refresh"), "Refresh();",QObject::tr("Refresh the block (e.g. updates display, recalculates, etc.).")));
   d->AddSubDocumentation(BuildScriptDocumentationBlockIOContainer());

   return d;
}

BlockSW::BlockSW(QJSEngine *se, Block *block) : DataClassSW(se, block)
{

}

void BlockSW::ApplyInputDataClass(const QJSValue &value)
{
   DataClass* input = m_dataClass->GetWorkspace()->FindInstance(value);
   if (input)
   {
      static_cast<Block*>(m_dataClass)->ApplyInputDataClass(input);
   }
}
QJSValue BlockSW::GetOutputs()
{
   return m_scriptEngine->newQObject(new BlockIOContainerSW(m_scriptEngine,&static_cast<Block*>(m_dataClass)->GetOutputs()));
}
QJSValue BlockSW::GetInputs()
{
   return m_scriptEngine->newQObject(new BlockIOContainerSW(m_scriptEngine,&static_cast<Block*>(m_dataClass)->GetInputs()));
}
void BlockSW::Refresh()
{
   static_cast<Block*>(m_dataClass)->Refresh();
}
}
