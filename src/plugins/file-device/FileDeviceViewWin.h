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

#include <QMainWindow>
#include <QAction>
#include "FileDevice.h"

QT_BEGIN_NAMESPACE
class QToolBar;
class QMainWindow;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QComboBox;
class QDoubleSpin;
QT_END_NAMESPACE

namespace terbit
{

class FileDeviceView;

class FileDeviceViewWin : public QMainWindow
{
   Q_OBJECT
public:
   explicit FileDeviceViewWin(FileDeviceView* parent);
   ~FileDeviceViewWin(){}
   void SetRunEnabled(bool en){m_playAct->setEnabled(en);}
   void SetStopEnabled(bool en){m_stopAct->setEnabled(en);}
   void SetPauseEnabled(bool en){m_pauseAct->setEnabled(en);}
   void SetSingleEnabled(bool en){m_singleBufferAct->setEnabled(en);}
   void SetLoopEnabled(bool en){m_loopAct->setEnabled(en);}
   bool GetLoopEnabled(void){return m_loopAct->isChecked();}
   void SetLoop(bool loop){m_loopAct->setChecked(loop);}

private slots:
signals:
public slots:

private:
   void addControls();
   void createActions();
   void createToolBars();
   FileDeviceView* m_parent;

   QToolBar *m_pbCtrlBar;
   QAction  *m_playAct;
   QAction  *m_pauseAct;
   QAction  *m_stopAct;
   QAction  *m_singleBufferAct;
   QAction  *m_loopAct;
   QMainWindow *m_qmainWin;
   QLineEdit   *m_editFilePathName;
   QPushButton *m_btnBrowse;
   QComboBox   *m_comboBoxType;
};

}
