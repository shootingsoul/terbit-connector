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
#include <QJSEngine>
#include <QDockWidget>
#include <QPushButton>
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QToolButton)

namespace terbit
{

class DataClass;

class WorkspaceDockWidget : public QDockWidget
{
   Q_OBJECT

public:
   WorkspaceDockWidget(DataClass* origin, const QString& title);

   DataClass* GetOrigin() { return m_origin; }

   void AssignUniqueObjectName();

signals:
private:
   DataClass* m_origin;
   QWidget* m_titleBar = NULL;
};

class WorkspaceDockWidgetSW : public QObject
{
   Q_OBJECT
public:
   WorkspaceDockWidgetSW(QJSEngine* se, WorkspaceDockWidget* dock);
   virtual ~WorkspaceDockWidgetSW() {}


protected:
   QJSEngine* m_scriptEngine;
   WorkspaceDockWidget* m_dock;
};



}
