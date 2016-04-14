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

#include "FileDeviceView.h"
#include "FileDeviceViewWin.h"
#include "FileDeviceViewAdvanced.h"
#include "ProgressLineEdit.h"
#include <QtWidgets>

namespace terbit
{

#define X(a, b) b,
static char *TerbitDataTypeStrs[] = { TERBIT_DATA_TYPES };
#undef X

FileDeviceView::FileDeviceView(terbit::FileDevice* pDvc) : terbit::WorkspaceDockWidget(pDvc, pDvc->GetName())
{
   m_qCtrlWin  = new FileDeviceViewWin(this);
   m_advDlg    = new FileDeviceViewAdvanced(this);
   m_btnBrowse = new QPushButton(tr("Browse..."));
   m_progEditFile  = new ProgressLineEdit(this);
   m_curFileLabel  = new QLabel(tr("Current file: "));

   QGridLayout *pGridLayout = new QGridLayout;
   pGridLayout->addWidget(new QLabel(tr("Input File")), 0, 0, 1, 1, 0);
   pGridLayout->addWidget(m_progEditFile, 1, 0, 1, 1, 0);
   pGridLayout->addWidget(m_btnBrowse, 1, 1, 1, 1, 0);
   pGridLayout->addWidget(m_curFileLabel, 2 ,0 ,2 , 1, 0);
   pGridLayout->addWidget(m_advDlg, 4, 0, 1, 2, 0);
   pGridLayout->addWidget(m_qCtrlWin, 5, 0, 1, 2, 0);



   m_qCtrlWin->setParent(this);

   //setTabOrder(m_progEditFile, m_btnBrowse);
   //setTabOrder(m_btnBrowse, m_advDlg);

   m_pDvc = pDvc;

   setupActions();

   this->setAcceptDrops(true);

   m_qCtrlWin->SetRunEnabled(true);
   m_qCtrlWin->SetStopEnabled(false);

   m_newNElts = true;
   m_newFile = true;
   if(m_pDvc)
   {
      m_nElts    = m_pDvc->GetNumElts();
      m_advDlg->GetBufSizeBox()->setValue(m_nElts);
      m_advDlg->GetFreqBox()->setValue(m_pDvc->GetFreq());
      m_advDlg->GetDataTypeBox()->setCurrentIndex((int)m_pDvc->GetDataType());
   }
   QWidget *wrapper = new QWidget();
   wrapper->setLayout(pGridLayout);
   //setLayout(pGridLayout);
   setWidget(wrapper);
}


void FileDeviceView::setupActions()
{
   connect(m_btnBrowse, SIGNAL(clicked()), this, SLOT(browse()));
   connect(m_pDvc, SIGNAL(ConfigUpdated()), this, SLOT(configUpdated()));
   connect(m_advDlg, SIGNAL(ByteAdjust()), this, SLOT(AdjustBytes()));
}

void FileDeviceView::browse()
{
   // Display file chooser
   QString str =
         QFileDialog::getOpenFileName(this, tr("Open Input File"), "", "");

   if(!str.isNull())
   {
      // Put path in edit box
      m_progEditFile->setText(str);
      if(str != m_fileName)
      {
         m_newFile = true;
      }
      m_fileName = str;
   }
}

void FileDeviceView::AdjustBytes()
{
   m_pDvc->SkipBytes(1);
}

// This function is called when the underlying device signals that it
// has taken on new parameters such as buf len, data type, freq, file, etc.
void FileDeviceView::configUpdated(void)
{

   if(NULL != m_pDvc)
   {
      QFileInfo file(m_pDvc->GetFilePathName()); // use QFile to extract filename from path

      if(!m_progEditFile->hasFocus() && m_progEditFile->text().isEmpty())
      {
         m_progEditFile->setText(m_pDvc->GetFilePathName());
      }

      if(!m_advDlg->GetBufSizeBox()->hasFocus())
      {
         m_advDlg->GetBufSizeBox()->setValue(m_pDvc->GetNumElts());
      }
      if(!m_advDlg->GetFreqBox()->hasFocus())
      {
         m_advDlg->GetFreqBox()->setValue(m_pDvc->GetFreq());
      }
      if(!m_advDlg->GetDataTypeBox())
      {
         m_advDlg->GetDataTypeBox()->setCurrentIndex(m_pDvc->GetDataType());
      }

      m_curFileLabel->setText(tr("Current file: %1").arg(file.fileName()));

      m_qCtrlWin->SetLoop(m_pDvc->GetLoop());
      setButtons(m_pDvc->GetMode());

      if(0 != m_pDvc->GetFileBytes())
      {
         m_progEditFile->SetProgress((double)m_pDvc->GetFilePos()/(double)m_pDvc->GetFileBytes() * 100.0);
         m_progEditFile->update();
      }
   }
}

void FileDeviceView::OnRun(void)
{
   bool localInit = true;
   TerbitDataType type = (TerbitDataType)m_advDlg->GetDataTypeBox()->currentIndex();

   if(m_pDvc)
   {
      if(m_newFile)
      {
         localInit = m_pDvc->UpdateFile(m_progEditFile->text());
         // TODO: if !localInit - tell user
         m_newFile = false;
      }
      m_pDvc->SetFreq(m_advDlg->GetFreqBox()->value());
      m_pDvc->SetLoop(m_qCtrlWin->GetLoopEnabled());
      m_pDvc->SetNumCh(1);
      m_pDvc->SetDataType(type);
      if(localInit)
      {
         m_pDvc->Start();
      }
   }
   m_lastCmd = FDCRun;
}

void FileDeviceView::OnPause(void)
{
   if(!m_pDvc->Pause())
   {
      ;
   }
   m_lastCmd = FDCPause;
}

void FileDeviceView::OnStop(void)
{
   if(!m_pDvc->Stop())
   {
      ;
   }
   m_lastCmd = FDCStop;
}

void FileDeviceView::OnSingle(void)
{
   bool localInit = true;
   TerbitDataType type = (TerbitDataType)m_advDlg->GetDataTypeBox()->currentIndex();
   if(m_newFile)
   {
      localInit = m_pDvc->UpdateFile(m_progEditFile->text());
      // TODO: if !localInit - tell user
      m_newFile = false;
   }
   m_pDvc->SetFreq(m_advDlg->GetFreqBox()->value());
   m_pDvc->SetLoop(m_qCtrlWin->GetLoopEnabled());
   m_pDvc->SetNumCh(1);
   m_pDvc->SetDataType(type);

   if(localInit && m_pDvc->Single())
   {
      ;
   }
   else
   {
      ;
   }
}
void FileDeviceView::OnLoop(void)
{
   m_pDvc->SetLoop(m_qCtrlWin->GetLoopEnabled());
}

void FileDeviceView::OnDataType(void)
{
   m_pDvc->UpdateSrc((TerbitDataType)m_advDlg->GetDataTypeBox()->currentIndex());

}

void FileDeviceView::OnFreq(void)
{
   m_pDvc->SetFreq(m_advDlg->GetFreqBox()->value());
}

void FileDeviceView::OnBufSize(void)
{
   size_t curNelts = m_advDlg->GetBufSizeBox()->value();
//   if(m_newNElts && localInit)
   {
      if(curNelts != m_nElts)
      {
         m_pDvc->UpdateSrc(curNelts);
         m_nElts = curNelts;
      }
      // TODO: if !localInit - tell user
      m_newNElts = false;
   }
}


void FileDeviceView::setButtons(FileDvcDPMode_t m)
{
   if(m != m_lastMode)
   {
      switch(m)
      {
      case FDMInitializing:
         m_qCtrlWin->SetStopEnabled(false);
         m_qCtrlWin->SetRunEnabled(false);
         m_qCtrlWin->SetPauseEnabled(false);
         m_qCtrlWin->SetLoopEnabled(false);
         m_qCtrlWin->SetSingleEnabled(false);
         m_advDlg->SetDataTypeEnabled(false);
         m_progEditFile->setEnabled(false);
         m_btnBrowse->setEnabled(false);
         m_advDlg->SetBufSizeEnabled(false);
         m_advDlg->SetFreqSpinEnabled(false);
         break;

      case FDMInitialized:
         m_qCtrlWin->SetStopEnabled(false);
         m_qCtrlWin->SetRunEnabled(true);
         m_qCtrlWin->SetPauseEnabled(false);
         m_qCtrlWin->SetLoopEnabled(true);
         m_qCtrlWin->SetSingleEnabled(true);
         m_advDlg->SetDataTypeEnabled(true);
         m_progEditFile->setEnabled(true);
         m_btnBrowse->setEnabled(true);
         m_advDlg->SetBufSizeEnabled(true);
         m_advDlg->SetFreqSpinEnabled(true);
         break;

      case FDMPaused:
         m_qCtrlWin->SetStopEnabled(true);
         m_qCtrlWin->SetRunEnabled(true);
         m_qCtrlWin->SetPauseEnabled(false);
         m_qCtrlWin->SetSingleEnabled(true);
         m_qCtrlWin->SetLoopEnabled(true);
         m_advDlg->SetDataTypeEnabled(true);// disabled makes it hard to see
         m_progEditFile->setEnabled(true);
         m_btnBrowse->setEnabled(false);
         m_advDlg->SetBufSizeEnabled(true);
         m_advDlg->SetFreqSpinEnabled(true);
         break;

      case FDMRunning:
         m_qCtrlWin->SetStopEnabled(true);
         m_qCtrlWin->SetRunEnabled(false);
         m_qCtrlWin->SetPauseEnabled(true);
         m_qCtrlWin->SetSingleEnabled(true);
         m_qCtrlWin->SetLoopEnabled(true);
         m_advDlg->SetDataTypeEnabled(true);
         m_progEditFile->setEnabled(true); // disabled makes it hard to see
         m_btnBrowse->setEnabled(false);
         m_advDlg->SetBufSizeEnabled(true);
         m_advDlg->SetFreqSpinEnabled(true);
         break;

      case FDMStopped:
         m_qCtrlWin->SetStopEnabled(false);
         m_qCtrlWin->SetRunEnabled(true);
         m_qCtrlWin->SetPauseEnabled(false);
         m_qCtrlWin->SetSingleEnabled(true);
         m_qCtrlWin->SetLoopEnabled(true);
         m_advDlg->SetDataTypeEnabled(true);
         m_progEditFile->setEnabled(true);
         m_btnBrowse->setEnabled(true);
         m_advDlg->SetBufSizeEnabled(true);
         m_advDlg->SetFreqSpinEnabled(true);
         break;

      case FDMTermd:
         m_qCtrlWin->SetStopEnabled(false);
         m_qCtrlWin->SetRunEnabled(false);
         break;

      default:
         break;
      }
   }
   m_lastMode = m;
}


void FileDeviceView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("text/uri-list"))
   {
      event->acceptProposedAction();
   }
}

void FileDeviceView::dropEvent(QDropEvent *event)
{
   const QMimeData *mimeData = event->mimeData();

   if(m_pDvc->GetMode() == FDMRunning || m_pDvc->GetMode() == FDMPaused || m_pDvc->GetMode() == FDMInitializing)
   {
      // throw message box about terminating current session
      QMessageBox msgBox;
      QPushButton *stopButton = msgBox.addButton(tr("Stop Device"), QMessageBox::ActionRole);
      QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
      msgBox.setText(tr("The device must be stopped to complete this action."));
      msgBox.setInformativeText(tr("Do you want to stop the device and configure it with this file?"));
      msgBox.setDefaultButton(QMessageBox::Cancel);
      msgBox.setIcon(QMessageBox::Warning);
      int ret = msgBox.exec(); ret; // eliminate warning
      if (msgBox.clickedButton() == stopButton)
      {
         int32_t timeout = 20;
         OnStop();
         while(0 < timeout-- && (m_pDvc->GetMode() != FDMStopped))
         {
            QThread::msleep(50);
         }
         if(m_pDvc->GetMode() != FDMStopped)
         {
            QMessageBox msgBoxF;
            msgBoxF.setText(tr("Failed to stop the device."));
            msgBoxF.setIcon(QMessageBox::Critical);
            msgBoxF.exec();
            return;
         }

      }
      else if (msgBox.clickedButton() == cancelButton)
      {
          // abort
         return;
      }
   }

   if (mimeData->hasUrls())
   {
     QList<QUrl> urlList = mimeData->urls();
     if(1 < urlList.size())
     {
        ; // message that we currently only accept one file
     }

     QString path = urlList.at(0).toLocalFile();
     if(!path.isNull())
     {
        // Put path in edit box
        m_progEditFile->setText(path);
        if(path != m_fileName)
        {
           m_newFile = true;
        }
        m_fileName = path;
     }
     m_progEditFile->setText(path);
   }
}

}
