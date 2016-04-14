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
#include <QDialog>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(QWebView)
QT_FORWARD_DECLARE_CLASS(QLineEdit)

namespace terbit
{

class ScriptDialogView : public QDialog
{
   Q_OBJECT
public:
   enum Style
   {
      STYLE_OK = 0,
      STYLE_CONFIRM = 1,
      STYLE_YESNO = 2,
      STYLE_OKCANCEL = 3,
      STYLE_INPUT = 4
   };

   ScriptDialogView(QWidget* parent, Style style, const QString& html, const QString& confirmText);

   const QVariant& ExecResult();

private slots:
   void OnConfirm();
   void OnInput();
   void OnOk();
   void OnCancel();

protected:
   void closeEvent(QCloseEvent * event);

private:
   bool ValidateConfirmation();

   QWebView *m_web;
   QLineEdit* m_input;
   bool m_closeByCode;
   QString m_confirmText;
   Style m_style;
   QVariant m_result;
};

}

