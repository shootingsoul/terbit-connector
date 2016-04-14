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

#include "Plugin.h"
#include "Workspace.h"
#include "LogDL.h"

namespace terbit
{

Plugin::Plugin(const QString& name, bool internal, const QString& fileName, const QString& directory, const QString& version, const QString& company, const QString& description)
   : m_name(name), m_internal(internal), m_fileName(fileName), m_directory(directory), m_version(version), m_provider(company), m_description(description)
{
   m_namespace = m_provider.toLower() + "." + m_name.toLower();
}

Plugin::~Plugin()
{
}

}
