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
#include <QSettings>

namespace terbit
{

class OptionsDL
{
public:
   OptionsDL(); //options stored in standard system location
   OptionsDL(const QString& settingsFile); //from given file

   ~OptionsDL();

   QString GetLastStartTimeUTC();
   void SetLastStartTimeUTC(const QString& time);

   bool GetDarkTheme();
   void SetDarkTheme(bool enabled);

   bool GetRestoreSessionEnabled();
   void SetRestoreSessionEnabled(bool enabled);

   QString GetLogFileName();
   void SetLogFileName(const QString& fileName);

   bool GetLastStartClean();
   void SetLastStartClean(bool clean);

   void GetRestoreSessionValues(QString& script);
   void SetRestoreSessionValues(const QString& script);

   void GetScriptFilesMRU(std::list<QString>& list);
   void SetScriptFilesMRU(const std::list<QString>& list);

   int GetDefaultFontSize();
   void SetDefaultFontSize(int size);

private:

   QSettings* m_user;
};


QString DetermineLastStartTime(OptionsDL* options);
bool UpdateLastStartTime(OptionsDL* options);

}
