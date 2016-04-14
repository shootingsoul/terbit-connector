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

/**
HOW TO USE: Logging

//logging can be done in one line . . .
LogInfo(g_log.general, tr("Silly function returned code: %1").arg(code));

//first parameter is the category.  categories are defined for the application/component you are coding in
LogInfo(g_log.plot, QString("Plotting for data source: %1 (%2)").arg(dataSourceName).arg(providerName));

//three pre-defined log levels with a macro for each
LogInfo(g_log.general, "This is fun.");
LogWarning(g_log.general, "This is getting serious.");
LogError(g_log.general, "What in tarnation is this?");

//If you have a really complicated log event, then use the log manager directly instead of the macros
if (g_logManager.IsActive(g_logLevels.info, g_log.general))
{
   QString msg;
   //...call lots of functions to build up msg
   g_logManager.Write(g_logLevels.info, g_log.general, msg);
}

//you can register new categories to log.
//NOTE: This must be done on program startup.
//store the categories in a global variable and give it a unique description (e.g. namespace style)
class DatalightLogCategories
{
public:
   uint32_t general;
   uint32_t plot;
};

extern DatalightLogCategories g_log;

//startup code to register categories globally
g_log.general = g_logManager.RegisterCategory("terbit.datalight.general");
g_log.plot = g_logManager.RegisterCategory("terbit.datalight.plot");

*/

#pragma once

#include <QString>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#endif // Q_MOC_RUN
#include <iostream>
#include <fstream>

namespace terbit
{

class LogRecord
{
public:
   LogRecord();

   uint32_t level;
   uint32_t category;
   boost::posix_time::ptime timeStamp;
   QString objName;
   QString message;
   QString threadId;
};

class LogManager;

class LogOutput
{
public:
   LogOutput();
   virtual ~LogOutput();
   virtual void OutputRecord(LogManager* logManager, const LogRecord& record) = 0;
   virtual void Start() = 0;
   virtual void Stop() = 0;
private:
};

class LogOutputConsole : public LogOutput
{
public:
   LogOutputConsole();
   void OutputRecord(LogManager* logManager, const LogRecord& record);
   void Start();
   void Stop();
};

class LogOutputFile : public LogOutput
{
public:
   LogOutputFile(const QString& fileName);
   void OutputRecord(LogManager* logManager, const LogRecord& record);
   void Start();
   void Stop();
private:
   QString m_fileName;
   std::ofstream m_file;
};

class LogManager
{
public:
   LogManager();
   ~LogManager();

   void Close();

   uint32_t RegisterLevel(const QString& levelName); //add or find existing . . .
   uint32_t RegisterCategory(const QString& categoryName); //add or find existing . . .
   void RegisterOutput(LogOutput* output);
   void UnregisterOutput(LogOutput* output);

   const QString& GetCategoryName(uint32_t category) { return m_categories[category].name; }
   const QString& GetLevelName(uint32_t level) { return m_levels[level].name; }

   bool IsActive(uint32_t level, uint32_t category)
   {
      //only write if output is active, otherwise log entries waste away in the buffer
      return (m_outputActive && (m_levels[level].active || m_categories[category].active));
   }

   //*** todo make level/categories inactive . . .

   void Write(uint32_t level, uint32_t category, const QString& message);
   void Write(uint32_t level, uint32_t category, const QString& objName, const QString& message);

private:
   class Area
   {
   public:
       QString name;
       bool active;
   };

   void StartOutput();
   void StopOutput();

   //threaded function call . . .
   void ProcessOutput();
   std::vector<Area> m_categories;
   std::vector<Area> m_levels;
   std::list<LogOutput*> m_outputs;
   boost::lockfree::queue<LogRecord*> m_buffer;
   boost::thread* m_outputThread;
   bool m_outputActive;
   bool m_useLocalTime;
};


//Global log for all libs and apps to use

extern LogManager g_logManager;

class LogLevels
{
public:
   LogLevels() {}
   uint32_t info;
   uint32_t warning;
   uint32_t error;
};

extern LogLevels g_logLevels;

#define LogBase(level, category, msg) if (g_logManager.IsActive(level, category)) { g_logManager.Write(level, category, msg); }
#define LogError(category, msg) LogBase(g_logLevels.error, category, msg)
#define LogInfo(category, msg) LogBase(g_logLevels.info, category, msg)
#define LogWarning(category, msg) LogBase(g_logLevels.warning, category, msg)

#define LogBase2(level, category, obj, msg) if (g_logManager.IsActive(level, category)) { g_logManager.Write(level, category, obj, msg); }
#define LogError2(category, obj, msg) LogBase2(g_logLevels.error, category, obj, msg)
#define LogInfo2(category, obj, msg) LogBase2(g_logLevels.info, category, obj, msg)
#define LogWarning2(category, obj, msg) LogBase2(g_logLevels.warning, category, obj, msg)

void LogGlobalStartup();
void LogGlobalShutdown();

}
