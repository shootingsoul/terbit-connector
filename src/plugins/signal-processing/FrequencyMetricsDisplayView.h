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

#include <QString>
#include "connector-core/WorkspaceDockWidget.h"
#include "tools/FrequencySignalMetrics.h"

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)
QT_FORWARD_DECLARE_CLASS(QRadioButton)


namespace terbit
{

class SigAnalysisProcessor;
class DataSet;

class FrequencyMetricsDisplayView: public QWidget
{
   Q_OBJECT
public:
   FrequencyMetricsDisplayView(SigAnalysisProcessor* proc);



private slots:
   void onDvcUpdate();
   void onAutoBitsPerSamp(int en);
   void onMeasUnitsChg();
   void onNSetsAvgChg(int n);
   //void onHarmExclChg(int n);
   void onNBinsHarmChg(int n);
   void onNBinsFundaChg(int n);
   void onNBinsDCChg(int n);
   void onNHarmsChg(int n);
   void onSampRateChg(double r);
   void onnBitsPerSampChg(int n);
   void onNoiseChg(double n);
private:
   FrequencyMetricsDisplayView(const FrequencyMetricsDisplayView& o); //disable copy ctor

   void SettingsUpdated();

   SigAnalysisProcessor* m_dvc;
   QComboBox* m_maxHarmonics;

   QSpinBox  *m_nHarmCalcSpin      = NULL;
   QSpinBox  *m_nBinsExclDCSpin    = NULL;
   QSpinBox  *m_nBinsExclFundaSpin = NULL;
   QSpinBox  *m_nBinsExclHarmSpin  = NULL;
   //QSpinBox  *m_SFDRHarmExclSpin   = NULL;
   //QSpinBox  *m_nSetsAvgSpin       = NULL;
   QSpinBox  *m_bitsPerSampSpin    = NULL;
   QDoubleSpinBox *m_noiseLvlSpin  = NULL;
   QComboBox *m_measUnitsCombo     = NULL;
   QCheckBox *m_bitsPerSampAutoCheck = NULL;
   QRadioButton **m_scaleBtnsAry   = NULL;

   void createWidgets();
};

}

