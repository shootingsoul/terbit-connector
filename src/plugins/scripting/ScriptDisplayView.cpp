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
#include <QtWebKitWidgets/QWebView>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QCloseEvent>

#include "ScriptDisplay.h"
#include "ScriptDisplayView.h"

namespace terbit
{

ScriptDisplayView::ScriptDisplayView(ScriptDisplay* sd) : WorkspaceDockWidget(sd,sd->GetName()), m_model(sd)
{
   auto l = new QVBoxLayout;
   m_web = new QWebView();
   l->addWidget(m_web);

   auto wrapper = new QWidget();
   wrapper->setLayout(l);
   setWidget(wrapper);
}

void ScriptDisplayView::OnModelUpdateHTML(const QString &html)
{
   m_web->setHtml(html);
}

void ScriptDisplayView::OnModelSetFloating(bool floating)
{
   this->setFloating(floating);
}

}
