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

#include "connector-core/WorkspaceDockWidget.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QRadioButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)


namespace terbit
{

class SigAnalysisProcessor;

class FFTProcessorView : public QWidget
{
   Q_OBJECT
public:
   FFTProcessorView(SigAnalysisProcessor* fft);
   ~FFTProcessorView();

   //void dragEnterEvent(QDragEnterEvent *event);
   //void dropEvent(QDropEvent *event);

private slots:
   void OnSamplingRateChanged(double);
   void OnAutoUpdateSamplingRateChanged(int);
   void OnRemoveDCChanged(int);
   void OnAdjustWindowChanged(bool);
   void OnWindowTypeChanged(int);
   void OnWindowLenChanged(int len);
   void OnWindowOptionChanged(double o);
   void onDvcUpdated();
   void onScaleChanged();

private:
   //void OnWindowChanged();
   void UpdateWindowTypeDisplay();
   void UpdateWindowLenDisplay();
   SigAnalysisProcessor* m_fft;
   QDoubleSpinBox* m_samplingRate;
   QCheckBox* m_autoUpdateSamplingRate;
   QCheckBox* m_removeDC;
   QComboBox* m_windowType;
   //QComboBox* m_scale;
   QRadioButton **m_scaleBtnsAry;
   QRadioButton* m_adjustWindow;
   QRadioButton* m_fixedWindow;
   QSpinBox* m_windowLen;
   QDoubleSpinBox* m_windowOption;
   QLabel* m_windowOptionLabel;
};

}
