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

#include "ScriptFilesMRU.h"
#include "OptionsDL.h"
#include "Workspace.h"
#include <QFile>
#include "tools/Tools.h"

namespace terbit
{

const int ScriptFilesMRU::MRU_SIZE = 10;

ScriptFilesMRU::ScriptFilesMRU(Workspace* workspace) : m_workspace(workspace), m_list()
{
   ReadMRU();
}

void ScriptFilesMRU::Add(const QString &scriptFileName)
{
   //very first file or file not the top of list, then add or re-org list
   if (m_list.size() == 0 || m_list.front() != scriptFileName)
   {
      //remove if already in the list, adding to the top
      m_list.remove(scriptFileName);

      m_list.push_front(scriptFileName);
      if (m_list.size() > MRU_SIZE)
      {
         m_list.pop_back();
      }
      WriteMRU();
   }
}

void ScriptFilesMRU::clear()
{
   if (m_list.size()>0)
   {
      m_list.clear();
      WriteMRU();
   }
}

void ScriptFilesMRU::ReadMRU()
{
   m_workspace->GetOptions()->GetScriptFilesMRU(m_list);

   //verify file still exists to keep in list . . .
   for(iterator i = begin(); i != end(); )
   {
      QFile f(*i);
      if (!f.exists())
      {
         i = m_list.erase(i);
      }
      else
      {
         ++i;
      }
   }
}

void ScriptFilesMRU::WriteMRU()
{
   m_workspace->GetOptions()->SetScriptFilesMRU(m_list);
}

}
