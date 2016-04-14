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

#include "Log.h"
#include <string>

namespace terbit
{

LogRecord::LogRecord()
{
}

LogOutput::LogOutput()
{
}

LogOutput::~LogOutput()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


LogOutputConsole::LogOutputConsole() : LogOutput()
{
}

void LogOutputConsole::OutputRecord(LogManager* logManager, const LogRecord& record)
{
   std::cout << boost::posix_time::to_iso_extended_string(record.timeStamp)
             << " " << record.threadId.toUtf8().constData()
             << " " << logManager->GetLevelName(record.level).toUtf8().constData()
             << " " << logManager->GetCategoryName(record.category).toUtf8().constData()
             << " \"" << record.objName.toUtf8().constData() << "\""
             << " \"" << record.message.toUtf8().constData() << "\""
             << std::endl;
}
void LogOutputConsole::Start()
{
}
void LogOutputConsole::Stop()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

LogOutputFile::LogOutputFile(const QString& fileName) : LogOutput(), m_fileName(fileName)
{
}

void LogOutputFile::OutputRecord(LogManager* logManager, const LogRecord& record)
{
   if (m_file.is_open())
   {
      QString message(record.message);
      QString objName(record.objName);
      message.replace("\"","\"\"");
      objName.replace("\"","\"\"");
      m_file << boost::posix_time::to_iso_extended_string(record.timeStamp)
                << ", " << record.threadId.toUtf8().constData()
                << ", " << logManager->GetLevelName(record.level).toUtf8().constData()
                << ", " << logManager->GetCategoryName(record.category).toUtf8().constData()
                << ", \"" << objName.toUtf8().constData() << "\""
                << ", \"" << message.toUtf8().constData() << "\""
                << std::endl;
   }
}
void LogOutputFile::Start()
{
   m_file.open(m_fileName.toUtf8().constData(), std::ios::out | std::ios::app);

}
void LogOutputFile::Stop()
{
   m_file.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

LogManager::LogManager() : m_buffer(100), m_outputActive(false)
{
   m_useLocalTime = true;
}

LogManager::~LogManager()
{
   Close();
}

void LogManager::Close()
{
   StopOutput();
}

void LogManager::StartOutput()
{
   if (m_outputs.size() > 0)
   {
      //call start on all outputs . . .
      for (std::list<LogOutput*>::iterator it=m_outputs.begin(); it != m_outputs.end(); ++it)
      {
         (*it)->Start();
      }

      m_outputActive = true;
      m_outputThread = new boost::thread(boost::bind(&LogManager::ProcessOutput, this));
   }
}

void LogManager::StopOutput()
{
   if (m_outputActive)
   {
      m_outputActive = false;
      m_outputThread->join();
      delete m_outputThread;

      //call stop on all outputs . . .
      for (std::list<LogOutput*>::iterator it=m_outputs.begin(); it != m_outputs.end(); ++it)
      {
         (*it)->Stop();
      }
   }
}

uint32_t LogManager::RegisterLevel(const QString& levelName)
{
   uint32_t res = m_levels.size();
   Area a;
   a.active = true;
   a.name = levelName;
   m_levels.push_back(a);
   return res;
}
uint32_t LogManager::RegisterCategory(const QString& categoryName)
{
   uint32_t res = m_categories.size();
   Area a;
   a.active = true;
   a.name = categoryName;
   m_categories.push_back(a);
   return res;
}

void LogManager::RegisterOutput(LogOutput* output)
{
   //full stop/start for registering outputs . . .
   //could use mutex instead, but this way we don't need to muxtex check when reading buffer in thread
   //moreover, output registration doesn't happen too often, so pay the stop/start price a few times
   //instead of mutex price a lot
   StopOutput();
   m_outputs.push_back(output);
   StartOutput();
}
void LogManager::UnregisterOutput(LogOutput* output)
{
   StopOutput();
   m_outputs.remove(output);
   StartOutput();
}

void LogManager::Write(uint32_t level, uint32_t category, const QString& message)
{
   LogRecord* record = new LogRecord();
   record->category = category;
   record->level = level;
   record->message = message;
   if (m_useLocalTime)
   {
      record->timeStamp = boost::posix_time::microsec_clock::local_time();
   }
   else
   {
      record->timeStamp = boost::posix_time::microsec_clock::universal_time();
   }
   record->threadId = QString::fromStdString(boost::lexical_cast<std::string>(boost::this_thread::get_id()));
   m_buffer.push(record);
}

void LogManager::Write(uint32_t level, uint32_t category, const QString &objName, const QString &message)
{
   LogRecord* record = new LogRecord();
   record->category = category;
   record->level = level;
   record->objName = objName;
   record->message = message;
   if (m_useLocalTime)
   {
      record->timeStamp = boost::posix_time::microsec_clock::local_time();
   }
   else
   {
      record->timeStamp = boost::posix_time::microsec_clock::universal_time();
   }
   record->threadId = QString::fromStdString(boost::lexical_cast<std::string>(boost::this_thread::get_id()));
   m_buffer.push(record);

}

void LogManager::ProcessOutput()
{
   //threaded function . . .
   LogRecord* record;
   while (m_outputActive || !m_buffer.empty())
   {
      boost::this_thread::sleep(boost::posix_time::milliseconds(400));
      while (m_buffer.pop(record))
      {
         //send record to outputs
         for (std::list<LogOutput*>::iterator it=m_outputs.begin(); it != m_outputs.end(); ++it)
         {
            (*it)->OutputRecord(this, *record);
         }
         delete record;
      }
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
LogManager g_logManager;

LogLevels g_logLevels;

void LogGlobalStartup()
{
   g_logLevels.error = g_logManager.RegisterLevel("error");
   g_logLevels.info = g_logManager.RegisterLevel("information");
   g_logLevels.warning = g_logManager.RegisterLevel("warning");
}

void LogGlobalShutdown()
{
   g_logManager.Close();
}


}
