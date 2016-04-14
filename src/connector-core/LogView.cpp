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

#include <QVBoxLayout>
#include "LogView.h"
#include "tools/Tools.h"
#include "Workspace.h"

namespace terbit
{

LogView::LogView(Workspace* workspace, LogManager* logManager) : WorkspaceDockWidget(NULL,tr("Log Viewer")), LogOutput(), m_workspace(workspace), m_logManager(logManager)
{
   m_table = new QTableWidget();
   m_table->setColumnCount(6);
   QStringList headers;
   headers << tr("Level") << tr("Message") << tr("Category") << tr("Object") << tr("Time") << tr("Thread")  ;
   m_table->setHorizontalHeaderLabels(headers);
   m_table->setColumnWidth(1,200);

   setWidget(m_table);

   m_logManager->RegisterOutput(this);
}
LogView::~LogView()
{
   m_logManager->UnregisterOutput(this);
}

const QEvent::Type LogViewNewRecordEvent::LogViewNewRecordEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

bool LogView::event(QEvent *evt)
{
   if (evt->type() == LogViewNewRecordEvent::LogViewNewRecordEventType)
   {
      LogViewNewRecordEvent* e = static_cast<LogViewNewRecordEvent*>(evt);

      //reverse chron display
      m_table->insertRow(0);

      m_table->setItem(0, 0, e->cells[0]);
      m_table->setItem(0, 1, e->cells[1]);
      m_table->setItem(0, 2, e->cells[2]);
      m_table->setItem(0, 3, e->cells[3]);
      m_table->setItem(0, 4, e->cells[4]);
      m_table->setItem(0, 5, e->cells[5]);

      return true;
   }

   return QDockWidget::event(evt);
}

void LogView::OutputRecord(LogManager* logManager, const LogRecord& record)
{
   //post event to gui thread, this is called from another thread . . .
   LogViewNewRecordEvent* e = new LogViewNewRecordEvent();

   e->cells[0] = new QTableWidgetItem( logManager->GetLevelName(record.level));
   e->cells[1] = new QTableWidgetItem( record.message);
   e->cells[2] = new QTableWidgetItem( logManager->GetCategoryName(record.category));
   e->cells[3] = new QTableWidgetItem( record.objName);
   e->cells[4] = new QTableWidgetItem( QString::fromStdString(boost::posix_time::to_iso_extended_string(record.timeStamp)));
   e->cells[5] = new QTableWidgetItem( record.threadId);

   QCoreApplication::postEvent(this,e);
}

void LogView::Start()
{
}

void LogView::Stop()
{
}

}
