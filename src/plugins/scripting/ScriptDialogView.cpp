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
#include <QtWebKitWidgets/QWebFrame>
#include <QApplication>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QLabel>
#include <QCloseEvent>
#include <QPushButton>

#include "ScriptDialogView.h"

namespace terbit
{

ScriptDialogView::ScriptDialogView(QWidget* parent, Style style, const QString& html, const QString& confirmText)
   : QDialog(parent), m_input(NULL), m_closeByCode(false), m_confirmText(confirmText), m_style(style)
{
   auto l = new QVBoxLayout;
   m_web = new QWebView();   


   //make background transparent by default and have default text color match application theme
   QPalette palette = m_web->palette();
   palette.setBrush(QPalette::Base, Qt::transparent);
   QColor c = QApplication::palette().color(QPalette::Text);
   palette.setColor(QPalette::Text, c);
   palette.setColor(QPalette::Foreground, c);
   m_web->page()->setPalette(palette);
   m_web->setAttribute(Qt::WA_OpaquePaintEvent, false);

   //fix the size, can't figure how to size to fit html
   m_web->setFixedSize(500,250);

   m_web->setHtml(html);

   //center the text by default through css
   //color is dynamic and css needs a file . . . so we update the style through javascript . . . sigh
   //m_web->settings()->setUserStyleSheetUrl(QUrl("qrc:///dialog.css"));
   QString jsCss(QString("document.fgColor = \"%1\"; document.body.style.textAlign = \"center\";").arg(QApplication::palette().color(QPalette::Text).name()));
   m_web->page()->mainFrame()->evaluateJavaScript(jsCss);

   l->addWidget(m_web);

   switch (style)
   {
   case STYLE_OK:
      {
         QDialogButtonBox* b = new QDialogButtonBox(this);
         b->addButton(QDialogButtonBox::Ok)->setAutoDefault(false);
         l->addWidget(b);
         connect(b, SIGNAL(accepted()), this, SLOT(OnOk()));
      }
      break;
   case STYLE_CONFIRM:
      {
         l->addWidget(new QLabel(tr("Please enter '%1' in the input below to continue").arg(m_confirmText)));
         m_input = new QLineEdit();
         l->addWidget(m_input);

         QDialogButtonBox* b = new QDialogButtonBox(this);
         b->addButton(QDialogButtonBox::Ok)->setAutoDefault(false);
         b->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);
         l->addWidget(b);
         connect(b, SIGNAL(accepted()), this, SLOT(OnConfirm()));
         connect(b, SIGNAL(rejected()), this, SLOT(OnCancel()));
      }
      break;
   case STYLE_INPUT:
      {
         m_input = new QLineEdit();
         l->addWidget(m_input);

         QDialogButtonBox* b = new QDialogButtonBox(this);
         b->addButton(QDialogButtonBox::Ok)->setAutoDefault(false);
         l->addWidget(b);
         connect(b, SIGNAL(accepted()), this, SLOT(OnInput()));
      }
      break;
   case STYLE_YESNO:
      {
         QDialogButtonBox* b = new QDialogButtonBox(this);
         b->addButton(QDialogButtonBox::Yes)->setAutoDefault(true);
         b->addButton(QDialogButtonBox::No)->setAutoDefault(false);
         l->addWidget(b);
         connect(b, SIGNAL(accepted()), this, SLOT(OnOk()));
         connect(b, SIGNAL(rejected()), this, SLOT(OnCancel()));
      }
      break;
   case STYLE_OKCANCEL:
      {
         QDialogButtonBox* b = new QDialogButtonBox(this);
         b->addButton(QDialogButtonBox::Ok)->setAutoDefault(true);
         b->addButton(QDialogButtonBox::Cancel)->setAutoDefault(false);
         l->addWidget(b);
         connect(b, SIGNAL(accepted()), this, SLOT(OnOk()));
         connect(b, SIGNAL(rejected()), this, SLOT(OnCancel()));
      }
      break;
   }

   setLayout(l);
   if (style == STYLE_INPUT || style == STYLE_CONFIRM)
   {
      m_input->setFocus();
   }
}

const QVariant& ScriptDialogView::ExecResult()
{
   setWindowModality(Qt::WindowModal);
   exec();
   return m_result;
}

void ScriptDialogView::OnConfirm()
{
   if (ValidateConfirmation())
   {
      m_result = true;
      m_closeByCode = true;
      close();
   }
}

void ScriptDialogView::OnInput()
{
   m_result = m_input->text();
   m_closeByCode = true;
   close();
}

void ScriptDialogView::OnOk()
{
   m_result = true;
   m_closeByCode = true;
   close();
}

void ScriptDialogView::OnCancel()
{
   m_result = false;
   m_closeByCode = true;
   close();
}

void ScriptDialogView::closeEvent(QCloseEvent *event)
{
   if (!m_closeByCode)
   {
      switch (m_style)
      {
      case STYLE_OK:
         event->accept();
         break;
      case STYLE_INPUT:
         m_result = m_input->text();
         event->accept();
         break;
      case STYLE_CONFIRM:
         if (ValidateConfirmation())
         {
            m_result = true;
            event->accept();
         }
         else
         {
            event->ignore();
         }
         break;
      case STYLE_OKCANCEL:
      case STYLE_YESNO:
         m_result = false;
         event->accept();
         break;
      }
   }
}

bool ScriptDialogView::ValidateConfirmation()
{
   bool res;
   if (m_confirmText == m_input->text())
   {
      res = true;
   }
   else
   {
      res = false;
      QMessageBox m(this);
      m.setText(tr("Please enter the correct input confirmation to continue."));
      m.exec();
      m_input->setFocus();
   }
   return res;
}

}

