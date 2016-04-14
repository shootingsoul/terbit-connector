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

#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include "OptionsDLView.h"
#include "OptionsDL.h"
#include "Workspace.h"
#include "StartupShutdownApp.h"
#include "BuildInfoCore.h"

namespace terbit
{

OptionsDLView::OptionsDLView(QWidget *parent, Workspace *workspace) : QDialog(parent), m_workspace(workspace)
{
   setWindowTitle(tr("%1 Options").arg(_STR_PRODUCT_NAME));

   QVBoxLayout* l = new QVBoxLayout();

   QGroupBox* theme = new QGroupBox(tr("Theme"));
   QHBoxLayout* themeLayout = new QHBoxLayout();

   m_themeStandard = new QRadioButton(tr("Standard"));
   m_themeStandard->setChecked(!m_workspace->GetOptions()->GetDarkTheme());
   themeLayout->addWidget(m_themeStandard);

   m_themeDark = new QRadioButton(tr("Dark"));
   m_themeDark->setChecked(m_workspace->GetOptions()->GetDarkTheme());
   themeLayout->addWidget(m_themeDark);

   theme->setLayout(themeLayout);
   l->addWidget(theme);


   m_restore = new QCheckBox(tr("Restore session on startup"),this);
   m_restore->setToolTip(tr("Check this box to direct %1 to save the configuration on shutdown and restore it on startup").arg(_STR_PRODUCT_NAME));
   m_restore->setChecked(m_workspace->GetOptions()->GetRestoreSessionEnabled());
   l->addWidget(m_restore);

   QHBoxLayout* layoutLog = new QHBoxLayout();
   QLabel *lbl = new QLabel("Log File");
   lbl->setToolTip(tr("File to which internal %1 log events will be saved").arg(_STR_PRODUCT_NAME));
   layoutLog->addWidget(lbl);
   m_logFile = new QLineEdit(m_workspace->GetOptions()->GetLogFileName());
   m_logFile->setToolTip(tr("File to which internal %1 log events will be saved").arg(_STR_PRODUCT_NAME));
   layoutLog->addWidget(m_logFile);
   QPushButton* selectLog = new QPushButton("...");
   selectLog->setToolTip(tr("Click to select log file name and location"));
   selectLog->setFixedSize(30,30);
   layoutLog->addWidget(selectLog);
   connect(selectLog,SIGNAL(pressed()),this, SLOT(OnSelectLogFile()));
   l->addLayout(layoutLog);

   QDialogButtonBox* b = new QDialogButtonBox(this);
   b->addButton(QDialogButtonBox::Ok);
   b->addButton(QDialogButtonBox::Cancel);
   l->addWidget(b);
   connect(b, SIGNAL(accepted()), this, SLOT(OnSave()));
   connect(b, SIGNAL(rejected()), this, SLOT(reject()));

   setLayout(l);
}

void OptionsDLView::OnSave()
{
   bool restore = m_restore->isChecked();
   m_workspace->GetOptions()->SetRestoreSessionEnabled(restore);

   bool darkTheme = m_themeDark->isChecked();
   if (darkTheme != m_workspace->GetOptions()->GetDarkTheme())
   {
      QMessageBox m(this);
      m.setText(tr("Restart the application to apply the new theme."));
      m.exec();
      m_workspace->GetOptions()->SetDarkTheme(darkTheme);
   }

   QString fileName = m_logFile->text();
   m_workspace->GetOptions()->SetLogFileName(fileName);
   //apply it now (will also clear if file name empty)
   ActivateAppLogFile(fileName);

   accept();
}

void OptionsDLView::OnSelectLogFile()
{
   QString fileName = QFileDialog::getSaveFileName(this,tr("Log File"));
   if (!fileName.isNull())
   {
      m_logFile->setText(fileName);
   }
}


}
