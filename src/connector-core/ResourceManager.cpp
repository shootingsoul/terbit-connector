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

#include "ResourceManager.h"

//resource init must be outside namespace and needed when used in a library
void TerbitResourceManagerInitializer()
{
   Q_INIT_RESOURCE(ConnectorCore);
}

namespace terbit
{

QString ResourceManager::GetLogo()
{
   //return QString(":/images/oscilloscope.png");
   return QString(":/images/connector_logo.png");
}

QString ResourceManager::GetTerbitLogo()
{
   return QString(":/images/terbit_logo_small.png");
}

QIcon ResourceManager::GetPlayerStartImage()
{
   return QIcon(":/images/media-playback-start-3.png");
}

QIcon ResourceManager::GetPlayerStopImage()
{
   return QIcon(":/images/media-playback-stop-3.png");
}



}
