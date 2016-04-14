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

#include <QString>
#include <list>

namespace terbit
{

   class Workspace;

   class ScriptFilesMRU
   {
   friend class Workspace;
   friend class ScriptProcessor;

   public:
      ScriptFilesMRU(Workspace* workspace);

      //iterator access through list style iterator
      typedef std::list<QString>::iterator iterator;
      typedef std::list<QString>::const_iterator const_iterator;

      iterator begin() { return m_list.begin(); }
      const_iterator begin() const { return m_list.begin(); }
      iterator end() { return m_list.end(); }
      const_iterator end() const { return m_list.end(); }

      size_t size() { return m_list.size(); }
      void clear();

   private:
      ScriptFilesMRU(const ScriptFilesMRU& o); //disable copy ctor

      //only let friends add files
      void Add(const QString& scriptFileName);


      void ReadMRU();
      void WriteMRU();

      static const int MRU_SIZE;

      Workspace* m_workspace;
      std::list<QString> m_list;

   };
}
