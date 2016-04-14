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

#include <QtWidgets>
#include <QDockWidget>
#include <QTableWidget>
#include <QEvent>
#include "tools/Log.h"
#include "WorkspaceDockWidget.h"

namespace terbit
{

class Workspace;

class LogViewNewRecordEvent : public QEvent
{
public:
   static const QEvent::Type LogViewNewRecordEventType;

   LogViewNewRecordEvent(): QEvent( LogViewNewRecordEvent::LogViewNewRecordEventType) {}
   QTableWidgetItem* cells[6];
};

class LogView: public WorkspaceDockWidget, public LogOutput
{
   Q_OBJECT
public:
   LogView(Workspace* workspace, LogManager* logManager);
   virtual ~LogView();

   //log output overrides
   void OutputRecord(LogManager* logManager, const LogRecord& record);
   void Start();
   void Stop();

   bool event(QEvent* evt);


private:
   LogView(const LogView& o); //disable copy ctor

   Workspace* m_workspace;
   LogManager* m_logManager;
   QTableWidget* m_table;

};

}
