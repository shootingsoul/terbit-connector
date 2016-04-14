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
#include <QString>

#include "connector-core/Block.h"

namespace terbit
{

class ScriptDisplayView;
class ScriptProcessor;

static const char* SCRIPTDISPLAY_TYPENAME = "display"; //already under scripting namespace

class ScriptDisplay : public Block
{
   Q_OBJECT
public:
   ScriptDisplay();
   virtual ~ScriptDisplay();

   virtual bool InteractiveInit();
   void Refresh();

   QObject* CreateScriptWrapper(QJSEngine* se);

   void SetProcessor(ScriptProcessor* processor);

   void ShowView();
   void CloseView();
   void UpdateHTML(const QString& html);
   void SetFloating(bool floating);
private slots:
   void OnViewClosed();
private:
   ScriptDisplayView* m_view;
   ScriptProcessor* m_processor;
   QString m_html;
   bool m_floating;
};

ScriptDocumentation *BuildScriptDocumentationScriptDisplay();

class ScriptDisplaySW : public BlockSW
{
   Q_OBJECT
public:
   ScriptDisplaySW(QJSEngine* se, ScriptDisplay* display);

   Q_INVOKABLE void SetHTML(const QString& html);
   Q_INVOKABLE void ShowWindow();

private:
   ScriptDisplay* m_display;
};


}
