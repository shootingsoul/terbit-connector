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
#include "SignalAnalysisView.h"
#include <QGridLayout>
#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QScrollArea>
#include <QStandardItemModel>
#include "tools/widgets/ExtensionWidget.h"
#include "FFTProcessorView.h"
#include "FrequencyMetricsDisplayView.h"
#include "MetricsValueView.h"
#include "connector-core/DataClass.h"

// Set this to 1 to make the signal processing control view GUI "scrollable".
#define SCROLL_SIGPROC_VIEW 1

namespace terbit
{

SignalAnalysisView::SignalAnalysisView(SigAnalysisProcessor *proc) : WorkspaceDockWidget(proc, proc->BuildPropertiesViewName()), m_proc(proc)
{

   ExtensionWidget* pext;
   QVBoxLayout *layout = new QVBoxLayout();

   setAcceptDrops(true);

   MetricsValueView *vals = new MetricsValueView(proc);
   pext = new ExtensionWidget(this, tr("Signal Analysis Data"), vals);
   pext->SetToolTipText(tr("Signal analysis values"));
   pext->SetCollapsed(false);
   layout->addWidget(pext);

   FrequencyMetricsDisplayView *metrx = new FrequencyMetricsDisplayView(proc);
   pext = new ExtensionWidget(this, tr("Signal Analysis Control"), metrx);
   pext->SetToolTipText(tr("Signal analysis options"));
   layout->addWidget(pext);

   FFTProcessorView *fft = new FFTProcessorView(proc);
   pext = new ExtensionWidget(this, tr("FFT Control"), fft);
   pext->SetToolTipText(tr("FFT options and controls"));
   layout->addWidget(pext);

   layout->addStretch(1);

   layout->setContentsMargins(0,0,0,0);


   QWidget *w = new QWidget();
   w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
   w->setLayout(layout);

#if  SCROLL_SIGPROC_VIEW
   QScrollArea *pScroll = new QScrollArea;
   pScroll->setWidget(w);
   pScroll->setWidgetResizable(true);
   setWidget(pScroll);
#else
   setWidget(w);
#endif // SCROLL SIGPROC_VIEW

   connect(m_proc, SIGNAL(NameChanged(DataClass*)), this, SLOT(onNameChanged(DataClass*)));
}

void SignalAnalysisView::onNameChanged(DataClass*)
{
   setWindowTitle(m_proc->BuildPropertiesViewName());
}

#if 1
void SignalAnalysisView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
   {
      event->acceptProposedAction();
   }
}

void SignalAnalysisView::dropEvent(QDropEvent *event)
{
   QStandardItemModel model;
   model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex());

   int numRows = model.rowCount();
   for (int row = 0; row < numRows; ++row)
   {
      QModelIndex index = model.index(row, 0);
      DataClassAutoId_t id = model.data(index, Qt::UserRole).toUInt();
      m_proc->ApplyInput(id);
   }
   event->acceptProposedAction();
}
#endif

}// terbit
