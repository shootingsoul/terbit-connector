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
#include "MetricsValueView.h"
#include "HarmonicsView.h"
#include "SigAnalysisProcessor.h"
#include <QLabel>
#include <QString>
#include <QGridLayout>
#include "tools/widgets/ExtensionWidget.h"

namespace terbit
{

static QLabel* makeDisp(QString initVal, QString tip);
static QLabel* makeLabel(QString lbl, QString tip);

MetricsValueView::MetricsValueView(terbit::SigAnalysisProcessor *proc) : m_dvc(proc)
{
   QGridLayout *layout = new QGridLayout();

   QString fundaFTip(tr("Calculated frequency of the fundamental harmonic."));
   QString fundaATip(tr("Calculated amplitude of the fundamental harmonic."));
   QString fundaBinTip(tr("Bin number of the fundamental harmonic."));
   QString harmoCtTip(tr("Number of harmonics detected in the data, or user-specified max."));
   QString snrTip(tr("Signal to noise ratio."));
   QString thdTip(tr("Total harmonic distortion."));
   QString sfdrTip(tr("Spurious-free dynamic range."));
   QString sinadTip(tr("Signal to noise and distortion."));
   QString enobTip(tr("Effective number of bits."));

   int row = 0;
   int col = 1;
   QLabel* lbl = makeLabel(tr("Freq: "), fundaFTip);
   m_fundaFreq = makeDisp(tr("NA"), fundaFTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_fundaFreq, row, col++, 1,1);
   col++; // spacer
   lbl = makeLabel(tr("Amp: "), fundaATip);
   m_fundaAmp = makeDisp(tr("NA"), fundaATip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_fundaAmp, row, col++, 1,1);

   row++;
   col = 1;
   lbl = makeLabel(tr("Bin: "), fundaBinTip);
   m_fundaBin = makeDisp(tr("NA"), fundaBinTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_fundaBin, row, col++, 1,1);
   col++; // spacer
   lbl = makeLabel(tr("SINAD: "), sinadTip);
   m_sinad = makeDisp(tr("NA"), sinadTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_sinad, row, col++, 1,1);

   row++;
   col = 1;
   lbl = makeLabel(tr("SNR: "), snrTip);
   m_snr = makeDisp(tr("NA"), snrTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_snr, row, col++, 1,1);
   col++; // spacer
   lbl = makeLabel(tr("THD: "), thdTip);
   m_thd = makeDisp(tr("NA"), thdTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_thd, row, col++, 1,1);

   row++;
   col = 1;
   lbl = makeLabel(tr("SFDR: "), sfdrTip);
   m_sfdr = makeDisp(tr("NA"), sfdrTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_sfdr, row, col++, 1,1);
   col++; // spacer
   lbl = makeLabel(tr("Harmonic Ct: "), harmoCtTip);
   m_harmonicCount = makeDisp(tr("NA"), harmoCtTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_harmonicCount, row, col++, 1,1);

   row++;
   col = 1;
   lbl = makeLabel(tr("ENOB: "), enobTip);
   m_enob = makeDisp(tr("NA"), enobTip);
   layout->addWidget(lbl, row, col++, 1,1);
   layout->addWidget(m_enob, row, col++, 1,1);
   col++; // spacer


   row++;
   col = 1;
   HarmonicsView *vals = new HarmonicsView(proc);
   ExtensionWidget* pext = new ExtensionWidget(this, tr("Harmonics"), vals);
   pext->SetToolTipText(tr("Frequency, amplitude, and bin for each harmonic."));
   layout->addWidget(pext, row, col, 1, 6);


   layout->setColumnStretch(0, 1);
   layout->setColumnStretch(3, 1);
   layout->setColumnStretch(6, 1);
   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
   setLayout(layout);

   onDvcUpdate();

   connect(m_dvc, SIGNAL(ProcUpdated()), this, SLOT(onDvcUpdate()));
}



void MetricsValueView::onDvcUpdate()
{
   QString scale = m_dvc->GetdBScale() == dBc?tr(" dBc"):tr(" dBFS");

   m_fundaFreq->setText(QString::number(m_dvc->GetFundaFreq()).append(" Hz"));
   m_fundaAmp->setText(QString::number(m_dvc->GetFundaAmp()).append(" dBFS"));
   m_fundaBin->setText(QString::number(m_dvc->GetFundaBin()));
   m_harmonicCount->setText(QString::number(m_dvc->GetHarmonicCount()));
   m_snr->setText(QString::number(m_dvc->GetSNR()).append(scale));
   m_thd->setText(QString::number(m_dvc->GetTHD()).append(scale));
   m_sfdr->setText(QString::number(m_dvc->GetSFDR()).append(scale));
   m_sinad->setText(QString::number(m_dvc->GetSINAD()).append(scale));
   m_enob->setText(QString::number(m_dvc->GetENOB()).append(scale));
}

////////////////////////////////////////////////////////////////////////////
/// \brief makeLabel
/// \param lbl
/// \param tip
/// \return
///
static QLabel* makeLabel(QString lbl, QString tip)
{
  QLabel *retVal = new QLabel(lbl);
  if(!tip.isEmpty() && !tip.isNull())
  {
   retVal->setToolTip(tip);
  }
  return retVal;
}

////////////////////////////////////////////////////////////////////////////
/// \brief makeDisp
/// \param initVal
/// \param tip
/// \return
///
static QLabel* makeDisp(QString initVal, QString tip)
{
   QLabel *retVal = new QLabel(initVal);
   int frameStyle = QFrame::Box;     // Box, Panel, StyledPanel
   int shadowStyle = QFrame::Raised; // Plain, Raised, Sunken

   if(!tip.isEmpty() && !tip.isNull())
   {
      retVal->setToolTip(tip);
   }
   retVal->setFrameStyle(frameStyle | shadowStyle);
   retVal->setTextInteractionFlags(Qt::TextSelectableByMouse);
   retVal->setAlignment(Qt::AlignRight);

   return retVal;
}


}// terbit
