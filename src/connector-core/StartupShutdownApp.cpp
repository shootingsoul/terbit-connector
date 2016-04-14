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

#include <QFile>
#include <QApplication>
#include <QMetaType>
#include "StartupShutdownApp.h"
#include "LogDL.h"
#include <tools/Tools.h>
#include "ResourceManager.h"
#include "plugins/scripting/ScriptingUtils.h"

namespace terbit
{
   LogOutputFile* g_appLogFile;
   LogOutputConsole* g_appLogConsole; //for debugg/dev mode

   void AppGlobalStartup()
   {
      g_appLogFile = NULL;
      g_appLogConsole = NULL;

      ToolsGlobalStartup();
      LogRegisterAppCategories();
      TerbitResourceManagerInitializer();
      ScriptingGlobalStartup();

      //so other types can be used in signals/slots
      qRegisterMetaType<DataClassAutoId_t>("DataClassAutoId_t");
   }

   void AppGlobalShutdown()
   {
      ScriptingGlobalShutdown();

      if (g_appLogFile)
      {
         g_logManager.UnregisterOutput(g_appLogFile);
         delete g_appLogFile;
         g_appLogFile = NULL;
      }
      if (g_appLogConsole)
      {
         g_logManager.UnregisterOutput(g_appLogConsole);
         delete g_appLogConsole;
         g_appLogConsole = NULL;
      }
      ToolsGlobalShutdown();

   }

   void ActivateAppLogFile(const QString &fileName)
   {
      if (g_appLogFile)
      {
         g_logManager.UnregisterOutput(g_appLogFile);
         delete g_appLogFile;
         g_appLogFile = NULL;
      }

      if (fileName.length() > 0)
      {
         g_appLogFile = new LogOutputFile(fileName);
         g_logManager.RegisterOutput(g_appLogFile);
      }
   }

   void ActivateAppLogConsole()
   {
      //when deugging/dev mode, output to console
      if (!g_appLogConsole)
      {
         g_appLogConsole = new LogOutputConsole();
         g_logManager.RegisterOutput(g_appLogConsole);
      }
   }
}
