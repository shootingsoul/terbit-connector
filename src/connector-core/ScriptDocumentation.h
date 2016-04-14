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

#include <map>
#include <QString>
#include <vector>
namespace terbit
{

class Scriptlet
{
public:
   Scriptlet(const QString& name, const QString& code, const QString& documentation) : m_name(name), m_code(code), m_documentation(documentation) {}
   const QString& GetName() { return m_name; }
   const QString& GetCode() { return m_code; }
   void SetCode(const QString& code) { m_code = code; }
   const QString& GetDocumentation() { return m_documentation; }
   void SetDocumentation(const QString& documentation) { m_documentation = documentation; }
private:
   QString m_name, m_code, m_documentation;
};

class ScriptDocumentation
{
public:
   ScriptDocumentation();
   ScriptDocumentation(const QString& documentationJSON);
   ~ScriptDocumentation();

   const QString& GetName() const { return m_name; }
   void SetName(const QString& name) { m_name = name; }

   const QString& GetSummary() const { return m_summary; }
   void SetSummary(const QString& summary) { m_summary = summary; }
   void AddScriptlet(Scriptlet* s);
   const std::vector<Scriptlet*> GetScriptlets() const { return m_scriptlets; }

   void AddSubDocumentation(ScriptDocumentation* doc);
   const std::vector<ScriptDocumentation*> GetSubDocumentation() const { return m_subDocumentation; }

   QString ToJSON() const;
   void LoadJSON(const QString& documenationJSON);
private:
   QString m_name;
   QString m_summary;
   std::vector<Scriptlet*> m_scriptlets;
   std::vector<ScriptDocumentation*> m_subDocumentation;
};

}
