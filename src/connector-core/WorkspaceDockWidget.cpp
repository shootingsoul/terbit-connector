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

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QApplication>

#include "WorkspaceDockWidget.h"
#include "DataClass.h"


namespace terbit
{

WorkspaceDockWidget::WorkspaceDockWidget(DataClass *origin, const QString &title) : QDockWidget(title), m_origin(origin)
{
   setAttribute(Qt::WA_DeleteOnClose);   
}


void WorkspaceDockWidget::AssignUniqueObjectName()
{
   //needed so we can restore dock settings
   //use window title in case there is no origin
   QString name;
   if (m_origin)
   {
      name = m_origin->GetUniqueId();
   }
   else
   {
      name = ":" + windowTitle();
   }

   setObjectName(name);
}

WorkspaceDockWidgetSW::WorkspaceDockWidgetSW(QJSEngine *se, WorkspaceDockWidget *dock) : m_scriptEngine(se), m_dock(dock)
{
}




}
