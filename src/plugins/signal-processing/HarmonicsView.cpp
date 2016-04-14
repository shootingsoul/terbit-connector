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
#include "HarmonicsView.h"
#include "SigAnalysisProcessor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

namespace terbit
{

HarmonicsView::HarmonicsView(SigAnalysisProcessor* proc) : m_dvc(proc), m_layout(NULL), m_mainLayout(NULL)
{
   if(m_dvc)
   {
      connect(m_dvc, SIGNAL(ProcUpdated()), this, SLOT(OnDvcUpdate()));
   }
   OnDvcUpdate();
}

void HarmonicsView::OnDvcUpdate()
{
   if (m_layout == NULL || m_layout->rowCount() != m_dvc->GetMaxHarmonics())
   {
      CreateGridStructure();
   }
   UpdateGridValues();
}

void HarmonicsView::CreateGridStructure()
{
   //clear existing layouts and controls
   if (m_layout)
   {
      while(m_layout->count())
      {
          QWidget* widget = m_layout->itemAt(0)->widget();
          if( widget )
          {
              m_layout->removeWidget(widget);
              delete widget;
          }
      }
      delete m_layout;
   }
   if (m_mainLayout)
   {
      delete m_mainLayout;
   }

   //build anew
   m_layout = new QGridLayout();

   m_layout->setHorizontalSpacing(20);

   //headers
   m_layout->addWidget(BuildHeaderLabel(tr("H"), tr("Harmonic Number")),0,0,Qt::AlignCenter);
   m_layout->addWidget(BuildHeaderLabel(tr("Bin"), tr("Bin number the harmonic resides in.")),0,1,Qt::AlignCenter);
   m_layout->addWidget(BuildHeaderLabel(tr("Freq"), tr("Frequency of the harmonic.")),0,2,Qt::AlignCenter);
   m_layout->addWidget(BuildHeaderLabel(tr("Amp"), tr("Amplitude of the harmonic.")),0,3,Qt::AlignCenter);

   if (m_dvc)
   {
      //rows with harmonic number and blank labels for up to max harmonics we'll get (skip fundamental)
      for(int i=1; i<m_dvc->GetMaxHarmonics(); ++i)
      {
         QLabel* lbl = BuildValueLabel();
         lbl->setText(tr("H%1").arg(QString::number(i+1)));
         m_layout->addWidget(lbl,i,0);
         m_layout->addWidget(BuildValueLabel(),i,1);
         m_layout->addWidget(BuildValueLabel(),i,2);
         m_layout->addWidget(BuildValueLabel(),i,3);
      }
   }

   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
   m_mainLayout = new QHBoxLayout();
   m_mainLayout->addLayout(m_layout);
   m_mainLayout->addStretch();
   setLayout(m_mainLayout);
}

QLabel* HarmonicsView::BuildHeaderLabel(const QString &text, const QString &toolTip)
{
   QLabel* lbl = new QLabel(text);
   lbl->setAlignment(Qt::AlignHCenter);
   lbl->setToolTip(toolTip);
   return lbl;
}

QLabel *HarmonicsView::BuildValueLabel()
{
   QLabel* lbl = new QLabel();
   lbl->setAlignment(Qt::AlignRight);
   return lbl;
}

void HarmonicsView::UpdateGridValues()
{
   QLabel* lbl;

   if (m_dvc)
   {
      //skip fundamental
      QString scale = (m_dvc->GetdBScale() == dBc?" dBc":" dBFS");
      int cnt = m_dvc->GetHarmonicCount();
      for(int i=1; i<cnt; ++i)
      {
         lbl = (QLabel*)m_layout->itemAtPosition(i,1)->widget();
         lbl->setText(QString::number(m_dvc->GetHarmIndex(i)));

         lbl = (QLabel*)m_layout->itemAtPosition(i,2)->widget();
         lbl->setText(QString::number(m_dvc->GetHarmFreq(i)));

         lbl = (QLabel*)m_layout->itemAtPosition(i,3)->widget();
         lbl->setText(QString::number(m_dvc->GetHarmAmp(i)).append(scale));
      }

      //clear out the other labels for unused harmonics
      for(int i = (cnt == 0 ? 1 : cnt); i<m_dvc->GetMaxHarmonics(); ++i)
      {
         lbl = (QLabel*)m_layout->itemAtPosition(i,1)->widget();
         lbl->setText("");

         lbl = (QLabel*)m_layout->itemAtPosition(i,2)->widget();
         lbl->setText("");

         lbl = (QLabel*)m_layout->itemAtPosition(i,3)->widget();
         lbl->setText("");
      }
   }
}

}// terbit
