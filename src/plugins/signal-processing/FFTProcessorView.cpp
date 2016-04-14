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
#include <limits>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QStandardItemModel>
#include <QMimeData>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "FFTProcessorView.h"
#include "FFTProcessor.h"
#include "SigAnalysisProcessor.h"

namespace terbit
{

#define X(a, b) b,
 static char *SigMtrxScaleStrs[] = { SIG_MTRX_SCALE_UNITS };
#undef X


FFTProcessorView::FFTProcessorView(SigAnalysisProcessor *fft) : QWidget(), m_fft(fft)
{
   QString sampRateTip(tr("Data sampling rate (Hz)"));
   QString windowTypeTip(tr("Type of window used in metrics processing."));
   QString autoUpdateTip(tr("Check to automatically update frequency from data set property \"SamplingRate\" if existing."));
   QString remDCTip(tr("Click to remove \"DC\" during FFT calculation."));
   QString fixedLenTip(tr("Click to enable a user defined window length."));
   QString varLenTip(tr("Click to automatically configure window length to length of input dataset."));
   QString windowOptionsTip(tr("Set Gaussian or Tukey windowing parameters."));
   QString winLenTip(tr("Set window length"));
   QString unitsTip(tr("Select dBc or dBFS scale."));

   m_samplingRate = new QDoubleSpinBox();
   m_samplingRate->setAlignment(Qt::AlignRight);
   m_samplingRate->setRange(1.0, 1.0995116e+12); // 2^40
   m_samplingRate->setMaximumWidth(150);
   m_samplingRate->setMinimumWidth(100);
   m_samplingRate->setToolTip(sampRateTip);
   m_samplingRate->setKeyboardTracking(false);

   m_autoUpdateSamplingRate = new QCheckBox(tr("Auto-Update"));
   m_autoUpdateSamplingRate->setToolTip(autoUpdateTip);

   m_removeDC = new QCheckBox(tr("Remove DC"));
   m_removeDC->setToolTip(remDCTip);

   m_windowType = new QComboBox();
   m_windowType->setToolTip(windowTypeTip);
   m_windowType->addItem(tr("None"),DisplayFFT::WINDOW_NONE);
   m_windowType->addItem(tr("Boxcar"),DisplayFFT::WINDOW_BOXCAR);
   m_windowType->addItem(tr("Gaussian"),DisplayFFT::WINDOW_GAUSSIAN);
   m_windowType->addItem(tr("Hamming"),DisplayFFT::WINDOW_HAMMING);
   m_windowType->addItem(tr("Hanning"),DisplayFFT::WINDOW_HANNING);
   m_windowType->addItem(tr("Triangle"),DisplayFFT::WINDOW_TRIANGLE);
   m_windowType->addItem(tr("Tukey"),DisplayFFT::WINDOW_TUKEY);

   m_adjustWindow = new QRadioButton(tr("Use Input Length"));
   m_adjustWindow->setToolTip(varLenTip);
   m_fixedWindow = new QRadioButton(tr("Fixed Length"));
   m_fixedWindow->setToolTip(fixedLenTip);

   m_windowLen = new QSpinBox();
   m_windowLen->setAlignment(Qt::AlignRight);
   m_windowLen->setMinimum(2);
   m_windowLen->setMaximum(0x7FFFFFFF);
   m_windowLen->setToolTip(winLenTip);

   m_windowOption = new QDoubleSpinBox();
   m_windowOption->setAlignment(Qt::AlignRight);
   //std::numeric_limits<double> dlim;
   m_windowOption->setRange(-1.0995116e+12, 1.0995116e+12); // 2^40
   m_windowOption->setMaximumWidth(150);
   m_windowOption->setMinimumWidth(100);
   m_windowOptionLabel = new QLabel();
   m_windowOption->setToolTip(windowOptionsTip);
   m_windowOptionLabel->setToolTip(windowOptionsTip);

   QVBoxLayout* l = new QVBoxLayout();

   QHBoxLayout* layout = new QHBoxLayout();
   QLabel *lbl = new QLabel(tr("Sampling Rate"));
   lbl->setToolTip(sampRateTip);
   layout->addWidget(lbl);
   layout->addWidget(m_samplingRate);
   layout->addWidget(m_autoUpdateSamplingRate);
   layout->addStretch();
   l->addLayout(layout);

   layout = new QHBoxLayout();
   layout->addWidget(m_removeDC);
   layout->addStretch(1);

   uint32_t nScales = dBGuard;

   if(0 != nScales)   {
      QGroupBox *scaleBox = new QGroupBox(tr("Scale"));
      QHBoxLayout *scaleLayout = new QHBoxLayout();

      scaleBox->setToolTip(tr("Decibel scale of FFT and analysis results"));

      m_scaleBtnsAry = new QRadioButton*[nScales];

      for(uint32_t i = 0; i < nScales; ++i)
      {
        m_scaleBtnsAry[i] = new QRadioButton(tr(SigMtrxScaleStrs[i]), scaleBox);
        scaleLayout->addWidget(m_scaleBtnsAry[i]);
      }
      scaleBox->setLayout(scaleLayout);
      layout->addWidget(scaleBox);
      layout->addStretch(1);
   }

   #if 0
   m_scale = new QComboBox();
   m_scale->setToolTip(unitsTip);
   for(uint32_t i = 0; i < sizeof(SigMtrxScaleStrs)/sizeof(SigMtrxScaleStrs[0]); ++i)
   {
      m_scale->addItem(SigMtrxScaleStrs[i], i);
   }
   layout->addWidget(m_scale);
   layout->addStretch();
#endif

   l->addLayout(layout);

   auto windowSizeBox = new QGroupBox(tr("Window"));
   windowSizeBox->setToolTip(tr("Select processing window sizing options."));
   auto windowSizeBoxLayout = new QVBoxLayout();

   layout = new QHBoxLayout();
   lbl = new QLabel(tr("Type"));
   lbl->setToolTip(windowTypeTip);
   layout->addWidget(lbl);
   layout->addWidget(m_windowType);
   layout->addStretch();
   windowSizeBoxLayout->addLayout(layout);

   layout = new QHBoxLayout();
   layout->addWidget(m_windowOptionLabel);
   layout->addWidget(m_windowOption);
   layout->addStretch();
   windowSizeBoxLayout->addLayout(layout);

   windowSizeBoxLayout->addWidget(m_adjustWindow);
   layout = new QHBoxLayout();
   layout->addWidget(m_fixedWindow);
   layout->addWidget(m_windowLen);
   layout->addStretch();
   windowSizeBoxLayout->addLayout(layout);

   windowSizeBox->setLayout(windowSizeBoxLayout);
   l->addWidget(windowSizeBox);

   l->addStretch();
   setLayout(l);

   m_windowLen->setValue(100);
   m_windowOption->setValue(0);
   onDvcUpdated();


   for(uint32_t i = 0; i < nScales; ++i)
   {
     connect(m_scaleBtnsAry[i], SIGNAL(clicked()), this, SLOT(onScaleChanged()));
   }
   connect(m_removeDC,SIGNAL(stateChanged(int)), this, SLOT(OnRemoveDCChanged(int)));
   connect(m_autoUpdateSamplingRate,SIGNAL(stateChanged(int)), this, SLOT(OnAutoUpdateSamplingRateChanged(int)));
   connect(m_samplingRate, SIGNAL(valueChanged(double)), this, SLOT(OnSamplingRateChanged(double)));
   connect(m_adjustWindow,SIGNAL(toggled(bool)),this,SLOT(OnAdjustWindowChanged(bool)));
   connect(m_windowType, SIGNAL(currentIndexChanged(int)),this, SLOT(OnWindowTypeChanged(int)));
   connect(m_windowLen, SIGNAL(valueChanged(int)), this, SLOT(OnWindowLenChanged(int)));
   connect(m_windowOption, SIGNAL(valueChanged(double)), this, SLOT(OnWindowOptionChanged(double)));
   connect(m_fft, SIGNAL(ProcUpdated()), this, SLOT(onDvcUpdated()));


}

void FFTProcessorView::onDvcUpdated()
{
   if(!m_samplingRate->hasFocus())
   {
      m_samplingRate->setValue(m_fft->GetSamplingRate());
   }

   if(!m_autoUpdateSamplingRate->hasFocus())
   {
      m_autoUpdateSamplingRate->setChecked(m_fft->GetAutoUpdateSamplingRate());
   }

   if(!m_removeDC->hasFocus())
   {
      m_removeDC->setChecked(m_fft->GetRemoveDC());
   }

   if(!m_windowType->hasFocus())
   {
      m_windowType->setCurrentIndex(m_windowType->findData(m_fft->GetWindowType()));
   }

   if(!m_adjustWindow->hasFocus())
   {
      m_adjustWindow->setChecked(m_fft->GetAdjustWindowToInputSize());
      m_fixedWindow->setChecked(!m_fft->GetAdjustWindowToInputSize());
   }

   if(!m_windowLen->hasFocus())
   {
      m_windowLen->setValue(m_fft->GetWindowLen());
   }

   if(!m_windowOption->hasFocus())
   {
      m_windowOption->setValue(m_fft->GetWindowOption());
   }

   if(!m_windowOption->hasFocus())
   {
      UpdateWindowTypeDisplay();
   }

   int32_t scale = m_fft->GetdBScale();
   if(scale < dBGuard)
   {
      m_scaleBtnsAry[scale]->setChecked(true);
   }
   else
   {
      ; // log error
   }

   UpdateWindowLenDisplay();

}

void FFTProcessorView::onScaleChanged()
{
   uint32_t i;

   for(i = 0; i < dBGuard; ++i)
   {
     if(m_scaleBtnsAry[i]->isChecked())
     {
        m_fft->SetdBScale((terbit::SigMtrxScaleUnits_t)i);
        break;
     }
   }
   if(i >= dBGuard)
   {
      ;// error - no buttons set.
   }
   else
   {
      m_fft->Refresh();
   }

}

FFTProcessorView::~FFTProcessorView()
{
}

#if 0
void FFTProcessorView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
   {
      event->acceptProposedAction();
   }
}

void FFTProcessorView::dropEvent(QDropEvent *event)
{
   QStandardItemModel model;
   model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex());

   int numRows = model.rowCount();
   for (int row = 0; row < numRows; ++row)
   {
      QModelIndex index = model.index(row, 0);
      DataClassAutoId_t id = model.data(index, Qt::UserRole).toUInt();
      //m_fft->ApplyInputDataClass(id);
   }
   event->acceptProposedAction();
}
#endif

void FFTProcessorView::OnSamplingRateChanged(double)
{   
   m_fft->SetSamplingRate(m_samplingRate->value());   
   m_fft->Refresh();
}

void FFTProcessorView::OnAutoUpdateSamplingRateChanged(int en)
{
   m_fft->SetAutoUpdateSamplingRate(Qt::Checked == en);
   m_fft->Refresh();
}

void FFTProcessorView::OnRemoveDCChanged(int en)
{
   m_fft->SetRemoveDC(Qt::Checked == en);
   m_fft->Refresh();
}

void FFTProcessorView::OnAdjustWindowChanged(bool)
{
   m_fft->SetAdjustWindowToInputSize(m_adjustWindow->isChecked());
   UpdateWindowLenDisplay();
   m_fft->Refresh();
}

void FFTProcessorView::OnWindowTypeChanged(int t)
{
   m_fft->SetWindow((DisplayFFT::WindowType)m_windowType->itemData(t).toInt(),m_fft->GetWindowLen(),m_fft->GetWindowOption());
   UpdateWindowTypeDisplay();
   m_fft->Refresh();
}

void FFTProcessorView::OnWindowLenChanged(int len)
{
   m_fft->SetWindow(m_fft->GetWindowType(),len,m_fft->GetWindowOption());
   m_fft->Refresh();
}

void FFTProcessorView::OnWindowOptionChanged(double v)
{
   m_fft->SetWindow(m_fft->GetWindowType(),m_fft->GetWindowLen(),v);
   m_fft->Refresh();
}

#if 0
void FFTProcessorView::OnWindowChanged()
{
   bool ok;
   size_t len = m_windowLen->text().toUInt(&ok);
   if (ok)
   {
      double option = m_windowOption->text().toDouble(&ok);
      if (ok)
      {
         m_fft->SetWindow((DisplayFFT::WindowType)m_windowType->currentData().toInt(),len,option);
         UpdateWindowOptionDisplay();
      }
      else
      {
         QMessageBox m(this);
         m.setText(tr("Invalid window option."));
         m.exec();
      }
   }
   else
   {      
      QMessageBox m(this);
      m.setText(tr("Invalid window length."));
      m.exec();
   }
}
#endif


void FFTProcessorView::UpdateWindowTypeDisplay()
{
   switch (m_fft->GetWindowType())
   {
   case DisplayFFT::WINDOW_GAUSSIAN:
      m_windowOption->setVisible(true);
      m_windowOptionLabel->setVisible(true);
      m_windowOptionLabel->setText(tr("alpha"));
      break;
   case DisplayFFT::WINDOW_TUKEY:
      m_windowOption->setVisible(true);
      m_windowOptionLabel->setVisible(true);
      m_windowOptionLabel->setText(tr("r"));
      break;
   default:
      m_windowOption->setVisible(false);
      m_windowOptionLabel->setVisible(false);
   }

}

void FFTProcessorView::UpdateWindowLenDisplay()
{
   if (m_adjustWindow->isChecked())
   {
      m_windowLen->setVisible(false);
   }
   else
   {
      m_windowLen->setVisible(true);
   }
}

}
