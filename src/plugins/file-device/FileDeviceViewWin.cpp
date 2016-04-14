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

#include "FileDeviceViewWin.h"
#include <QtWidgets>
#include <QSize>
#include <QGridLayout>
#include "tools/Tools.h"
#include <QList>
#include "ProgressLineEdit.h"
#include "FileDeviceViewAdvanced.h"

namespace terbit
{


FileDeviceViewWin::FileDeviceViewWin(FileDeviceView* parent): m_parent(parent)
{
   createActions();
   addControls();
   createToolBars();
   statusBar()->setSizeGripEnabled( false );
   statusBar()->showMessage(tr("Ready"));
}


void FileDeviceViewWin::addControls()
{
#if 0
   //QVBoxLayout* pLayout = new QVBoxLayout;
   QWidget* pWidget = new QWidget;
   QGridLayout *pGridLayout = new QGridLayout;
   ProgressLineEdit *pLineEdit = new ProgressLineEdit(this);
   pLineEdit->SetProgress(80);
   pGridLayout->addWidget(new QLabel(tr("Input File")), 0, 0, 1, 1, 0);
   pGridLayout->addWidget(pLineEdit, 1,0,1,1,0);
   pGridLayout->addWidget(new QPushButton("Browse..."), 1, 1, 1, 1, 0);
   pGridLayout->addWidget(new FileDvcDP_Adv(this), 2, 0, 1, 2, 0);
   //pLayout->addWidget(m_qmainWin);
   pWidget->setLayout(pGridLayout);
   //setLayout(pGridLayout);
   setCentralWidget(pWidget);

#endif
}


void FileDeviceViewWin::createActions()
{
   QList<QKeySequence> shortCutList;

   m_playAct = new QAction(QIcon(":/images/media-playback-start-3.png"), tr("&Start"), this);
   m_playAct->setShortcut(QKeySequence(tr("F5")));
   m_playAct->setStatusTip(tr("Begin sending file data (F5)"));
   connect(m_playAct, SIGNAL(triggered()), reinterpret_cast<QObject*>(m_parent), SLOT(OnRun()));

   m_pauseAct = new QAction(QIcon(":/images/media-playback-pause-3.png"), tr("&Pause"), this);
   //m_pauseAct->setShortcuts(QKeySequence:);
   m_pauseAct->setStatusTip(tr("Suspend reading and sending of file data"));
   connect(m_pauseAct, SIGNAL(triggered()), reinterpret_cast<QObject*>(m_parent), SLOT(OnPause()));

   m_stopAct = new QAction(QIcon(":/images/media-playback-stop-3.png"), tr("Stop"), this);
   m_stopAct->setShortcut(QKeySequence(tr("Shift+F5")));
   m_stopAct->setStatusTip(tr("Terminate reading and sending of file data (Shift-F5)"));
   connect(m_stopAct, SIGNAL(triggered()), reinterpret_cast<QObject*>(m_parent), SLOT(OnStop()));

   m_singleBufferAct = new QAction(QIcon(":/images/media-skip-forward-3.png"), tr("&Single"), this);
//   shortCutList.push_back(QKeySequence::Forward);
//   shortCutList.push_back(QKeySequence::MoveToNextPage);
//   m_singleBufferAct->setShortcuts(shortCutList);
//   shortCutList.clear();   
   m_singleBufferAct->setShortcut(QKeySequence(tr("F10")));
   m_singleBufferAct->setStatusTip(tr("Read and send one buffer full of data (F10)"));
   connect(m_singleBufferAct, SIGNAL(triggered()), reinterpret_cast<QObject*>(m_parent), SLOT(OnSingle()));

   m_loopAct = new QAction(QIcon(":/images/media-playlist-repeat.png"), tr("&Repeat"), this);
   m_loopAct->setStatusTip(tr("Continuously loop input data"));
   m_loopAct->setCheckable(true);
   connect(m_loopAct, SIGNAL(triggered()), reinterpret_cast<QObject*>(m_parent), SLOT(OnLoop()));
}

void FileDeviceViewWin::createToolBars()
{
   QSize iconSize;

   iconSize.setHeight(32);
   iconSize.setWidth(32);

   m_pbCtrlBar = addToolBar(tr("Playback Controls"));
   m_pbCtrlBar->setIconSize(iconSize);
   m_pbCtrlBar->setAllowedAreas(Qt::BottomToolBarArea);
   m_pbCtrlBar->addAction(m_stopAct);
   m_pbCtrlBar->addAction(m_playAct);
   m_pbCtrlBar->addAction(m_pauseAct);
   m_pbCtrlBar->addAction(m_singleBufferAct);
   m_pbCtrlBar->addSeparator();
   m_pbCtrlBar->addAction(m_loopAct);
}

}
