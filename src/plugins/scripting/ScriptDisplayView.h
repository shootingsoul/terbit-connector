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

#include <QObject>
#include <connector-core/WorkspaceDockWidget.h>

QT_FORWARD_DECLARE_CLASS(QWebView)
QT_FORWARD_DECLARE_CLASS(QLineEdit)

namespace terbit
{

class ScriptDisplay;

class ScriptDisplayView : public WorkspaceDockWidget
{
   Q_OBJECT
public:
   ScriptDisplayView(ScriptDisplay* sd);

public slots:
   void OnModelUpdateHTML(const QString& html);
   void OnModelSetFloating(bool floating);

private:
   QWebView *m_web;
   ScriptDisplay* m_model;
};

}
