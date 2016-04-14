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
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QStandardItemModel>
#include <QGroupBox>
#include <QMimeData>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRadioButton>
#include "SigAnalysisProcessor.h"
#include "FrequencyMetricsDisplayView.h"
//#include "FrequencyMetricsDisplay.h"
#include "connector-core/DataSet.h"

namespace terbit
{

#define X(a, b) b,
static char *TgtMetrxMeasUnitStrs[] = { SIG_MTRX_SCALE_UNITS };
#undef X


FrequencyMetricsDisplayView::FrequencyMetricsDisplayView(SigAnalysisProcessor *proc) : QWidget(), m_dvc(proc)
{
   QGridLayout *layout = new QGridLayout;
   QHBoxLayout *hl = new QHBoxLayout();

   QString nBitsTip(tr("Number of bits per sample."));
   QString nBitsAutoTip(tr("Check to automatically update the number of bits from the input data property \"BitsPerSample\", if existing."));
   QString unitsTip(tr("Select dBc or dBFS scale."));
   QString noiseTip(tr("Set the noise level used in harmonics calculations."));
   QString nHarmsTip(tr("Number of harmonics to detect and TBD."));
   QString exclsTip(tr("Number of bins and harmonics to exclude from calculations."));
   QString dcTip(tr("Number of bins to exclude from DC calculation."));
   QString fundaTip(tr("Number of bins to exclude from Fundamental frequency calculation."));
   QString harmTip(tr("Number of bins to exclude from harmonics calculation."));
   //QString harmSFDRTip(tr("Number of harmonics to exclude from SFDR calculation."));


   createWidgets();


   int row = 0;
   int col = 1;   
   int localRow = 0;
   int localCol = 0;
   QLabel* lbl = new QLabel(tr("Sample Bits"));
   lbl->setToolTip(nBitsTip);
   m_bitsPerSampSpin->setToolTip(nBitsTip);
   hl->addWidget(lbl);
   hl->addWidget(m_bitsPerSampSpin);
   m_bitsPerSampAutoCheck->setToolTip(nBitsAutoTip);
   hl->addWidget(m_bitsPerSampAutoCheck);
   lbl = new QLabel(tr("Auto"));
   lbl->setToolTip(nBitsAutoTip);
   hl->addWidget(lbl);
   hl->addStretch(1);

   uint32_t nScales = dBGuard;

   if(0 != nScales)   {
      QGroupBox *scaleBox = new QGroupBox(tr("Scale"));
      QHBoxLayout *scaleLayout = new QHBoxLayout();

      scaleBox->setToolTip(tr("Decibel scale of FFT and analysis results"));

      m_scaleBtnsAry = new QRadioButton*[nScales];

      for(uint32_t i = 0; i < nScales; ++i)
      {
        m_scaleBtnsAry[i] = new QRadioButton(tr(TgtMetrxMeasUnitStrs[i]), scaleBox);
        scaleLayout->addWidget(m_scaleBtnsAry[i]);
      }
      scaleBox->setLayout(scaleLayout);
      hl->addWidget(scaleBox);
      hl->addStretch(1);
   }

   //lbl = new QLabel(tr("Scale"));
   //layout->addWidget(lbl, localRow, col++, 1,1);
   //m_measUnitsCombo->setToolTip(unitsTip);
   //hl->addWidget(m_measUnitsCombo);
   //hl->addStretch(1);

   QWidget* w = new QWidget;
   w->setLayout(hl);
   layout->addWidget(w, row, col++, 1, 6 );

   row++;
   col = 1;

   //lbl = new QLabel(tr("nSets Avg"));
   //layout->addWidget(lbl, row, col++, 1,1);
   //layout->addWidget(m_nSetsAvgSpin, row, col++, 1,1);

   lbl = new QLabel(tr("Noise"));
   lbl->setToolTip(noiseTip);
   layout->addWidget(lbl, row, col++, 1,1);
   m_noiseLvlSpin->setToolTip(noiseTip);
   layout->addWidget(m_noiseLvlSpin,row, col++,1,1);
   col++;
   lbl = new QLabel(tr("nHarms"));
   lbl->setToolTip(nHarmsTip);
   m_nHarmCalcSpin->setToolTip(nHarmsTip);
   layout->addWidget(lbl, row, col++, 1,1);   
   layout->addWidget(m_nHarmCalcSpin, row, col++, 1,1);

   row++;
   col = 1;
   localRow = 0;
   localCol = 0;
   QGridLayout *gLayout = new QGridLayout;
   QGroupBox *group = new QGroupBox(tr("Exclusions"));
   group->setToolTip(exclsTip);
   lbl = new QLabel(tr("DC Bins"));
   lbl->setToolTip(dcTip);
   m_nBinsExclDCSpin->setToolTip(dcTip);
   gLayout->addWidget(lbl, localRow, localCol++, 1,1);
   gLayout->addWidget(m_nBinsExclDCSpin, localRow, localCol++,1,1);

   localCol++; // spacer

   lbl = new QLabel(tr("Funda Bins"));
   lbl->setToolTip(fundaTip);
   m_nBinsExclFundaSpin->setToolTip(fundaTip);
   gLayout->addWidget(lbl, localRow, localCol++, 1, 1);
   gLayout->addWidget(m_nBinsExclFundaSpin, localRow, localCol++, 1,1);

   localCol = 0;
   localRow++;
   lbl = new QLabel(tr("Harm Bins"));
   lbl->setToolTip(harmTip);
   m_nBinsExclHarmSpin->setToolTip(harmTip);
   gLayout->addWidget(lbl, localRow, localCol++, 1,1);
   gLayout->addWidget(m_nBinsExclHarmSpin, localRow, localCol++,1,1);

   localCol++; // spacer

   //lbl = new QLabel(tr("SFDR Harms"));
   //lbl->setToolTip(harmSFDRTip);
   //m_SFDRHarmExclSpin->setToolTip(harmSFDRTip);
   //gLayout->addWidget(lbl, localRow, localCol++, 1, 1);
   //gLayout->addWidget(m_SFDRHarmExclSpin, localRow, localCol++, 1,1);

   gLayout->setColumnStretch(2, 1);
   group->setLayout(gLayout);
   layout->addWidget(group, row, col, 1, 5);

   layout->setColumnStretch(0, 1);
   layout->setColumnStretch(3, 1);
   layout->setColumnStretch(6, 1);
   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);


   setLayout(layout);
   onDvcUpdate();

   for(uint32_t i = 0; i < nScales; ++i)
   {
     connect(m_scaleBtnsAry[i], SIGNAL(clicked()), this, SLOT(onMeasUnitsChg()));
   }
   connect(m_nHarmCalcSpin,    SIGNAL(valueChanged(int)), this, SLOT(onNHarmsChg(int)));
   connect(m_nBinsExclDCSpin,   SIGNAL(valueChanged(int)), this, SLOT(onNBinsDCChg(int)));
   connect(m_nBinsExclFundaSpin, SIGNAL(valueChanged(int)), this, SLOT(onNBinsFundaChg(int)));
   connect(m_nBinsExclHarmSpin, SIGNAL(valueChanged(int)), this, SLOT(onNBinsHarmChg(int)));
   //connect(m_SFDRHarmExclSpin, SIGNAL(valueChanged(int)), this, SLOT(onHarmExclChg(int)));
   //connect(m_nSetsAvgSpin,    SIGNAL(valueChanged(int)), this, SLOT(onNSetsAvgChg(int)));
   connect(m_measUnitsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onMeasUnitsChg()));
   connect(m_bitsPerSampAutoCheck, SIGNAL(stateChanged(int)), this, SLOT(onAutoBitsPerSamp(int)));
   connect(m_bitsPerSampSpin, SIGNAL(valueChanged(int)), this, SLOT(onnBitsPerSampChg(int)));
   connect(m_noiseLvlSpin, SIGNAL(valueChanged(double)), this, SLOT(onNoiseChg(double)));
   connect(m_dvc, SIGNAL(ProcUpdated()), this, SLOT(onDvcUpdate()));
}

void FrequencyMetricsDisplayView::onNoiseChg(double n)
{
   m_dvc->SetNoiseLvl(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onnBitsPerSampChg(int n)
{
   m_dvc->SetnBitsSamp(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onSampRateChg(double r)
{
   m_dvc->SetSamplingRate(r);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onNHarmsChg(int n)
{
   m_dvc->SetMaxHarmonics(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onNBinsDCChg(int n)
{
   m_dvc->SetBinsExclDC(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onNBinsFundaChg(int n)
{
   m_dvc->SetBinsExclFunda(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onNBinsHarmChg(int n)
{
   m_dvc->SetBinsExclHarm(n);
   m_dvc->Refresh();
}

//void FrequencyMetricsDisplayView::onHarmExclChg(int n)
//{
//   m_dvc->SetHarmExclSFDR(n);
//}

void FrequencyMetricsDisplayView::onNSetsAvgChg(int n)
{
   m_dvc->SetnSetsAvg(n);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::onMeasUnitsChg()
{
   uint32_t i;

   for(i = 0; i < dBGuard; ++i)
   {
     if(m_scaleBtnsAry[i]->isChecked())
     {
        m_dvc->SetdBScale((terbit::SigMtrxScaleUnits_t)i);
        break;
     }
   }
   if(i >= dBGuard)
   {
      ;// error - no buttons set.
   }
   else
   {
      m_dvc->Refresh();
   }
}

void FrequencyMetricsDisplayView::onAutoBitsPerSamp(int en)
{
   m_dvc->UpdateBitsPerSampleFromDataSet(Qt::Checked == en);
   m_dvc->Refresh();
}

void FrequencyMetricsDisplayView::createWidgets()
{

   m_measUnitsCombo = new QComboBox();
   for(uint32_t i = 0; i < sizeof(TgtMetrxMeasUnitStrs)/sizeof(TgtMetrxMeasUnitStrs[0]); ++i)
   {
      m_measUnitsCombo->addItem(TgtMetrxMeasUnitStrs[i], i);
   }

   m_nHarmCalcSpin = new QSpinBox(this);
   m_nHarmCalcSpin->setRange(0, 100000000);
   m_nHarmCalcSpin->setSingleStep(1);
   m_nHarmCalcSpin->setAccelerated(true);
   m_nHarmCalcSpin->setToolTip(tr("Number of harmonics to use in calculation display"));

   m_nBinsExclDCSpin = new QSpinBox(this);
   m_nBinsExclDCSpin->setRange(0, 100);
   m_nBinsExclDCSpin->setSingleStep(1);
   m_nBinsExclDCSpin->setAccelerated(true);

   m_nBinsExclFundaSpin = new QSpinBox(this);
   m_nBinsExclFundaSpin->setRange(0, 100);
   m_nBinsExclFundaSpin->setSingleStep(1);
   m_nBinsExclFundaSpin->setAccelerated(true);

   m_nBinsExclHarmSpin = new QSpinBox(this);
   m_nBinsExclHarmSpin->setRange(0, 100);
   m_nBinsExclHarmSpin->setSingleStep(1);
   m_nBinsExclHarmSpin->setAccelerated(true);

  // m_SFDRHarmExclSpin = new QSpinBox(this);
  // m_SFDRHarmExclSpin->setRange(0, 100);
  // m_SFDRHarmExclSpin->setSingleStep(1);
  // m_SFDRHarmExclSpin->setAccelerated(true);

   //m_nSetsAvgSpin = new QSpinBox(this);
   //m_nSetsAvgSpin->setRange(0, 100);
   //m_nSetsAvgSpin->setSingleStep(1);
   //m_nSetsAvgSpin->setAccelerated(true);

   m_bitsPerSampSpin = new QSpinBox(this);
   m_bitsPerSampSpin->setRange(1,1024);
   m_bitsPerSampSpin->setSingleStep(1);
   m_bitsPerSampSpin->setAccelerated(false);

   m_noiseLvlSpin = new QDoubleSpinBox(this);
   m_noiseLvlSpin->setRange(-1000, 1000);
   m_noiseLvlSpin->setSingleStep(1);
   m_noiseLvlSpin->setAccelerated(false);

   m_bitsPerSampAutoCheck = new QCheckBox(this);


}

void FrequencyMetricsDisplayView::onDvcUpdate()
{

   if(!m_nHarmCalcSpin->hasFocus())
   {
      m_nHarmCalcSpin->setValue(m_dvc->GetMaxHarmonics());
   }

   if(!m_nBinsExclDCSpin->hasFocus())
   {
      m_nBinsExclDCSpin->setValue(m_dvc->GetBinsExclDC());
   }

   if(!m_nBinsExclFundaSpin->hasFocus())
   {
      m_nBinsExclFundaSpin->setValue(m_dvc->GetBinsExclFunda());
   }

   if(!m_nBinsExclHarmSpin->hasFocus())
   {
      m_nBinsExclHarmSpin->setValue(m_dvc->GetBinsExclHarm());
   }

   //if(!m_SFDRHarmExclSpin->hasFocus())
   //{
   //   m_SFDRHarmExclSpin->setValue(m_dvc->GetHarmExclSFDR());
   //}

   //if(!m_nSetsAvgSpin->hasFocus())
   //{
   //   m_nSetsAvgSpin->setValue(m_dvc->GetnSetsAvg());
   //}


   if(!m_bitsPerSampSpin->hasFocus())
   {
      m_bitsPerSampSpin->setValue(m_dvc->GetnBitsSamp());
   }

   //if(!m_measUnitsCombo->hasFocus())
   //{
   //   m_measUnitsCombo->setCurrentIndex((int)m_dvc->GetdBScale());
   // }

   if(!m_noiseLvlSpin->hasFocus())
   {
      m_noiseLvlSpin->setValue(m_dvc->GetNoiseLvl());
   }

   int32_t scale = m_dvc->GetdBScale();
   if(scale < dBGuard)
   {
      m_scaleBtnsAry[scale]->setChecked(true);
   }
   else
   {
      ; // log error
   }

   if(!m_bitsPerSampAutoCheck->hasFocus())
   {
      m_bitsPerSampAutoCheck->setChecked(m_dvc->GetUpdateBitsPerSampleFromDataSet());
   }

}

#if 0
void FrequencyMetricsDisplayView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
   {
      event->acceptProposedAction();
   }
}

void FrequencyMetricsDisplayView::dropEvent(QDropEvent *event)
{
   QStandardItemModel model;
   model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex());

   int numRows = model.rowCount();
   for (int row = 0; row < numRows; ++row)
   {
      QModelIndex index = model.index(row, 0);
      DataClassAutoId_t id = model.data(index, Qt::UserRole).toUInt();
      m_dvc->ApplyInputDataClass(id);
   }
   event->acceptProposedAction();
}
#endif


}

