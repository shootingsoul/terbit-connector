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

#include <QObject>
#include <QJSValue>

namespace terbit
{

class FrequencySignalMetrics;
class Workspace;

class FrequencyMetricsLibSW : public QObject
{
   Q_OBJECT
public:
   FrequencyMetricsLibSW(Workspace* workspace);
   ~FrequencyMetricsLibSW();

   //calulations options
   Q_INVOKABLE void SetMaxHarmonicCount(int value) { m_maxHarmonicCount = value; }
   Q_INVOKABLE int GetMaxHarmonicCount() { return m_maxHarmonicCount; }

   Q_INVOKABLE void SetBinsDC(int value) { m_binsDC = value; }
   Q_INVOKABLE int GetBinsDC() { return m_binsDC; }

   Q_INVOKABLE void SetBinsFundamental(int value) { m_binsFundamental = value; }
   Q_INVOKABLE int GetBinsFundamental() { return m_binsFundamental; }

   Q_INVOKABLE void SetBinsHarmonics(int value) { m_binsHarmonics = value; }
   Q_INVOKABLE int GetBinsHarmonics() { return m_binsHarmonics; }

   Q_INVOKABLE void SetFullScale(bool value) { m_fullScale = value; }
   Q_INVOKABLE int GetFullScale() { return m_fullScale; }

   Q_INVOKABLE void SetBits(int value) { m_bits = value; }
   Q_INVOKABLE int GetBits() { return m_bits; }

   Q_INVOKABLE void SetNoiseRange(double decibels) { m_noiseDb = decibels; }
   Q_INVOKABLE int GetNoiseRange() { return m_noiseDb; }

   Q_INVOKABLE void Calculate(const QJSValue& dataSet);

   //calculation results
   Q_INVOKABLE double GetFundamentalFrequency();
   Q_INVOKABLE int GetHarmonicCount();

   Q_INVOKABLE double GetSNR();
   Q_INVOKABLE double GetTHD();
   Q_INVOKABLE double GetSFDR();
   Q_INVOKABLE double GetSINAD();
   Q_INVOKABLE double GetENOB();

private:
  FrequencySignalMetrics* m_metrics;
  double* m_fftDb = NULL;
  size_t m_fftDbLen = 0;
  double m_fundamental;
  Workspace* m_workspace;
  int m_maxHarmonicCount = 6;
  int m_binsDC = 0;
  int m_binsFundamental = 0;
  int m_binsHarmonics = 0;
  int m_bits = 12;
  bool m_fullScale = false;
  double m_noiseDb = 9;
};


}
