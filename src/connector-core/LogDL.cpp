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

#include "LogDL.h"

namespace terbit
{

//datalight logging category globals
AppLogCategories g_log;

void LogRegisterAppCategories()
{
    g_log.general = g_logManager.RegisterCategory("terbit.connector.general");
    g_log.startup = g_logManager.RegisterCategory("terbit.connector.startup");
    g_log.shutdown = g_logManager.RegisterCategory("terbit.connector.shutdown");
}

}



