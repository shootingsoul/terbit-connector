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

#include <QObject>
#include <QDateTime>
#include <cmath>
#include <QApplication>
#include <QtPlugin>
#include "stdint.h"
#include <tools/Tools.h>
#include <connector-core/StartupShutdownApp.h>
#include <connector-core/BuildInfoCore.h>
#include <connector-core/Workspace.h>
#include <connector-core/LogDL.h>
#include <connector-core/OptionsDL.h>

Q_IMPORT_PLUGIN(MicrophoneFactory)
Q_IMPORT_PLUGIN(FileDeviceFactory)
Q_IMPORT_PLUGIN(SignalProcessingFactory)
Q_IMPORT_PLUGIN(DisplaysFactory)
Q_IMPORT_PLUGIN(PlotsFactory)
Q_IMPORT_PLUGIN(ScriptingFactory)

using namespace terbit;


int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   int res = 1;

   try
   {
      AppGlobalStartup();

#ifdef DEBUG
      //output log to console for debug/dev mode
      ActivateAppLogConsole();
#endif
      OptionsDL* options = new OptionsDL();

      ActivateAppLogFile(options->GetLogFileName());

      //ensure last start time is correct, update over internet if need be
      UpdateLastStartTime(options);

      //dynamically allocate so we can control when it's destroyed
      Workspace* workspace = new Workspace(options);

      workspace->Startup(a);
      LogInfo(g_log.startup,QObject::tr("%1 %2 build: %3").arg(_STR_PRODUCT_VERSION).arg(CompilerOptions()).arg(BUILD_ID_STR));

      //run the gui app main thread . . .
      res = a.exec();

      //shut 'em down
      //make sure workspace/app stuff is gone first so it can log whatever it wants to until the bitter end
      delete workspace;
      delete options;

   }
   catch (const std::exception& ex)
   {
      LogError(g_log.general,QString::fromUtf8(ex.what()));
   }
   catch (const std::string& ex)
   {
      LogError(g_log.general,QString::fromStdString(ex));
   }
   catch (...)
   {
      LogError(g_log.general,QObject::tr("Unknown exception"));
   }

   //shutdown category is good enough for now . . .
   LogInfo(g_log.shutdown,"");

   AppGlobalShutdown();

   dmsg("clean exit");

   return res;
}
