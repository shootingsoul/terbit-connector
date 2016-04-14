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
#include <vector>

namespace terbit
{

class Workspace;

class Plugin
{
public:

   Plugin(const QString& name, bool internal, const QString& fileName, const QString& directory, const QString& version, const QString& provider, const QString& description);
   ~Plugin();

   const QString& GetName() const { return m_name; }
   const QString& GetFileName() const { return m_fileName; }
   const QString& GetDirectory() const { return m_directory; }
   const QString& GetVersion() const { return m_version; }
   const QString& GetProvider() const { return m_provider; }
   const QString& GetDescription() const { return m_description; }
   const QString& GetNamespace() const { return m_namespace; }


private:
   Plugin(const Plugin& o); // disable copy ctor

   bool m_internal;
   QString m_name, m_fileName, m_directory, m_version, m_provider, m_description, m_namespace;
};

typedef std::vector<Plugin*> PluginList;

}
