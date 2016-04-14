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

#include <QDateTime>
#include "OptionsDL.h"

namespace terbit
{

OptionsDL::OptionsDL()
{
   //NOTE: always user UserScope . . . SystemScope requires extra permission end users may not have
   m_user = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Terbit", "Connector");
}

OptionsDL::OptionsDL(const QString &settingsFile)
{
   m_user = new QSettings(settingsFile, QSettings::IniFormat);
}

OptionsDL::~OptionsDL()
{
   delete m_user;
}

QString OptionsDL::GetLastStartTimeUTC()
{
   return m_user->value("LastStartTimeUTC","").toString();
}

void OptionsDL::SetLastStartTimeUTC(const QString &time)
{
   return m_user->setValue("LastStartTimeUTC", time);
}

bool OptionsDL::GetDarkTheme()
{
   return m_user->value("DarkTheme",false).toBool();
}

void OptionsDL::SetDarkTheme(bool enabled)
{
   m_user->setValue("DarkTheme",enabled);
}

bool OptionsDL::GetRestoreSessionEnabled()
{
   //restore session by default for global option
   return m_user->value("RestoreSessionEnabled",true).toBool();
}

void OptionsDL::SetRestoreSessionEnabled(bool enabled)
{
   m_user->setValue("RestoreSessionEnabled",enabled);
}

QString OptionsDL::GetLogFileName()
{
   return m_user->value("LogFileName","").toString();
}

void OptionsDL::SetLogFileName(const QString &fileName)
{
   m_user->setValue("LogFileName", fileName);
}

bool OptionsDL::GetLastStartClean()
{
   return m_user->value("LastStartClean",true).toBool();
}

void OptionsDL::SetLastStartClean(bool clean)
{
   return m_user->setValue("LastStartClean", clean);
}

void OptionsDL::GetRestoreSessionValues(QString &script)
{
   script = m_user->value("RestoreScript","").toString();
}

void OptionsDL::SetRestoreSessionValues(const QString &script)
{
   m_user->setValue("RestoreScript",script);
}

void OptionsDL::GetScriptFilesMRU(std::list<QString> &list)
{
   int size = m_user->beginReadArray("ScriptFiles");
   for (int i = 0; i < size; ++i)
   {
      m_user->setArrayIndex(i);
      list.push_back(m_user->value("fileName").toString());
   }
   m_user->endArray();
}

void OptionsDL::SetScriptFilesMRU(const std::list<QString> &list)
{
   m_user->beginWriteArray("ScriptFiles");
   std::list<QString>::const_iterator it = list.begin();
   for (size_t i = 0; i < list.size(); ++i, ++it)
   {
      m_user->setArrayIndex(i);
      m_user->setValue("fileName", *it);
   }
   m_user->endArray();
}


int OptionsDL::GetDefaultFontSize()
{
   return m_user->value("DefaultFontSize",12).toInt();
}

void OptionsDL::SetDefaultFontSize(int size)
{
   m_user->setValue("DefaultFontSize",size);
}


//Logical last start time
QString DetermineLastStartTime(OptionsDL* options)
{
   QDateTime lastStartTime;
   QString lastStartTimeString;
   lastStartTimeString = options->GetLastStartTimeUTC();
   if (lastStartTimeString.length() > 0)
   {
      lastStartTime = QDateTime::fromString(lastStartTimeString, Qt::ISODate);
   }
   else
   {
      lastStartTime = QDateTime::currentDateTimeUtc();
      lastStartTimeString = lastStartTime.toString(Qt::ISODate);
   }

   return lastStartTimeString;
}

//Logical update of last start time
bool UpdateLastStartTime(OptionsDL* options)
{
   bool res = true;

   QDateTime lastStartTime;
   QString lastStartTimeString;
   lastStartTimeString = options->GetLastStartTimeUTC();
   if (lastStartTimeString.length() > 0)
   {
      lastStartTime = QDateTime::fromString(lastStartTimeString, Qt::ISODate);
   }
   else
   {
      lastStartTime = QDateTime::currentDateTimeUtc();
   }

   QDateTime now = QDateTime::currentDateTimeUtc();

   if (res)
   {
      options->SetLastStartTimeUTC(now.toString(Qt::ISODate));
   }

   return res;
}


}
