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
#include "ScriptDisplay.h"
#include "ScriptDisplayView.h"
#include "ScriptProcessor.h"
#include <connector-core/Workspace.h>

namespace terbit
{


ScriptDisplay::ScriptDisplay() : Block(), m_view(NULL), m_processor(NULL), m_floating(false)
{
}

ScriptDisplay::~ScriptDisplay()
{
   //ensure view is closed
   if (m_view)
   {
      disconnect(m_view,SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      delete m_view;
   }
}

bool ScriptDisplay::InteractiveInit()
{
   ShowView();
   return true;
}

void ScriptDisplay::Refresh()
{
   if (m_view)
   {
      m_view->update(); //update is thread-safe
   }
}

QObject *ScriptDisplay::CreateScriptWrapper(QJSEngine *se)
{
   return new ScriptDisplaySW(se,this);
}

void ScriptDisplay::SetProcessor(ScriptProcessor *processor)
{
   m_processor = processor;
}

void ScriptDisplay::ShowView()
{
   if (m_view)
   {
      m_view->raise();
   }
   else
   {
      m_view = new ScriptDisplayView(this);
      m_view->OnModelUpdateHTML(m_html);
      connect(m_view,SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);
      if (m_floating)
      {
         GetWorkspace()->FloatDockWidgetOutside(m_view);
      }
      else
      {
         m_view->setFocus();
      }
   }
}

void ScriptDisplay::CloseView()
{
   if (m_view)
   {
      disconnect(m_view,SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      delete m_view;
      m_view = NULL;

      //remove ourself
      GetWorkspace()->DeleteInstance(this->GetAutoId());
   }
}

void ScriptDisplay::OnViewClosed()
{
   m_view = NULL;

   //remove ourself
   GetWorkspace()->DeleteInstance(this->GetAutoId());
}

void ScriptDisplay::UpdateHTML(const QString &html)
{
   m_html = html;
   if (m_view)
   {
      m_view->OnModelUpdateHTML(html);
   }
}

void ScriptDisplay::SetFloating(bool floating)
{
   m_floating = true;
   if (m_view)
   {
      m_view->OnModelSetFloating(floating);
   }
}


ScriptDocumentation *BuildScriptDocumentationScriptDisplay()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("HTML display window."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetHTML"), "SetHTML(html);",QObject::tr("Sets the html string for display.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowWindow"), "ShowWindow();",QObject::tr("Opens the window with the HTML display.")));

   return d;
}

ScriptDisplaySW::ScriptDisplaySW(QJSEngine *se, ScriptDisplay *display) : BlockSW(se,display), m_display(display)
{

}

void ScriptDisplaySW::SetHTML(const QString &html)
{
   m_display->UpdateHTML(html);
}

void ScriptDisplaySW::ShowWindow()
{
   emit m_display->ShowView();
}


}
