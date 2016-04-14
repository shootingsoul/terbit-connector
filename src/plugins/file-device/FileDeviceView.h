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

#include <QWidget>
#include "FileDevice.h"
#include "connector-core/WorkspaceDockWidget.h"

QT_BEGIN_NAMESPACE
class QAction;
class QToolBar;
class QMainWindow;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

namespace terbit
{
class ProgressLineEdit;
class FileDeviceViewAdvanced;
class FileDeviceViewWin;

class FileDeviceView : public terbit::WorkspaceDockWidget
{
   Q_OBJECT


public:
   explicit FileDeviceView(terbit::FileDevice* pDvc = NULL);

public slots:
   void OnRun(void);
   void OnPause(void);
   void OnLoop(void);
   void OnSingle(void);
   void OnStop(void);
   void OnDataType(void);
   void OnFreq(void);
   void OnBufSize(void);
   void AdjustBytes(void);

private slots:
   void browse();
   void configUpdated(void);


signals:   
public slots:

protected:
   void dragEnterEvent(QDragEnterEvent *event);
   void dropEvent(QDropEvent *event);

private:
   void setupActions(void);
   void setButtons(terbit::FileDvcDPMode_t m);
   terbit::FileDvcDPMode_t m_lastMode;
   terbit::FileDevice*     m_pDvc;
   terbit::FileDvcDPCmd_t  m_lastCmd;
   QString         m_fileName;
   size_t          m_nElts;

   ProgressLineEdit  *m_progEditFile; // combo edit box/progress bar
   QPushButton       *m_btnBrowse;    // Browse button to go with edit
   FileDeviceViewAdvanced     *m_advDlg;       // Advanced options collapsible
   FileDeviceViewWin *m_qCtrlWin;     // A Widget with Toolbar
   QLabel            *m_curFileLabel;

   bool m_newNElts;
   bool m_newFile;
};

}
