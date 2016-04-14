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

#include "connector-core/Block.h"
#include "tools/Tools.h"
#include "tools/DisplayFFT.h"
#include "tools/FrequencySignalMetrics.h"
#include <QMutex>


namespace terbit
{


#define SIG_MTRX_SCALE_UNITS      \
    X(dBc,   "dBc")   \
    X(dBFS,  "dBFS")  \
    X(dBGuard, "N/A")

#define X(a, b) a,
enum SigMtrxScaleUnits_t { SIG_MTRX_SCALE_UNITS };
#undef X

// now, when you need string representations, add the following code
// to the .c file where it is needed:
// #define X(a, b) b,
// static char *SigMtrxScaleStrs[] = { SIG_MTRX_SCALE_UNITS };
// #undef X


class DataSet;
//class DisplayFFT;
class FrequencySignalMetrics;

static const char* SIG_ANALYSIS_PROCESSOR_TYPENAME = "signal-analysis";

class SigAnalysisProcessor : public Block
{
   Q_OBJECT

   friend class SigAnalysisProcSW;

public:
   SigAnalysisProcessor();
   ~SigAnalysisProcessor();

   const static BlockIOCategory_t OUTPUT_FFT;

   bool ShowPropertiesView();
   void ClosePropertiesView();
   QString BuildPropertiesViewName();
   bool Init();
   bool InteractiveInit();
   QObject* CreateScriptWrapper(QJSEngine* se);
   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);
   void SetName(const QString &name);
   void ApplyInputDataClass(DataClass* dc);
   void SetDataSet(DataSet* buf);

   void Refresh();
   void UpdateFrequencyXValues();
   bool UpdateBuffers();
   DisplayFFT* GetFFT(){return m_fft;}
   FrequencySignalMetrics* GetMetrics(){return m_sigMetrx;}


   // FFT Functions
   void UpdateSampleRateFromDataSet(bool en){m_autoUpdateSamplingRate = en;}


   void SetSamplingRate(double samplingRate);
   double GetSamplingRate(void){return m_samplingRateHz;}
   void SetAutoUpdateSamplingRate(bool en);
   bool GetAutoUpdateSamplingRate(){return m_autoUpdateSamplingRate;}
   //void SetSamplingBits(uint32_t nBits){m_nBitsSample = nBits; }
   //uint32_t GetSamplingBits(void){return m_nBitsSample;}

   bool GetRemoveDC(){return m_fft->GetRemoveDC();}
   void SetRemoveDC(bool value){m_fft->SetRemoveDC(value);}
   bool GetAdjustWindowToInputSize(){return m_adjustWindowToInputSize; }
   bool SetAdjustWindowToInputSize(bool value);

   DisplayFFT::WindowType GetWindowType() { return m_fft->GetWindowType(); }
   double GetWindowOption() { return m_fft->GetWindowOption(); }
   size_t GetWindowLen() { return m_fft->GetWindowLen(); }
   void SetWindow(DisplayFFT::WindowType type, size_t len, double option);

   // Metrics Functions
   double GetFundaFreq();
   uint32_t GetFundaBin();
   double GetFundaAmp();
   int GetHarmonicCount(){return m_sigMetrx->GetHarmonics().size();}
   size_t GetHarmIndex(uint32_t harm);
   double GetHarmAmp(uint32_t harm);
   double GetHarmFreq(uint32_t harm);

   double GetSNR(){return m_sigMetrx->GetSNR();}
   double GetTHD(){return m_sigMetrx->GetTHD();}
   double GetSFDR(){return m_sigMetrx->GetSFDR();}
   double GetSINAD(){return m_sigMetrx->GetSINAD();}
   double GetENOB(){return m_sigMetrx->GetENOB();}

   void UpdateBitsPerSampleFromDataSet(bool en){m_autoUpdateBitsPerSamp = en;}
   bool GetUpdateBitsPerSampleFromDataSet(){return m_autoUpdateBitsPerSamp;}
   const int GetMaxHarmonics() { return m_maxHarmonics; }
   void SetMaxHarmonics(int max){m_maxHarmonics = max;}
   uint32_t GetBinsExclDC(){return m_nBinsExclDC;}
   void SetBinsExclDC(uint32_t nBins){m_nBinsExclDC = nBins;}
   uint32_t GetBinsExclFunda(){return m_nBinsExclFunda;}
   void SetBinsExclFunda(uint32_t nBins){m_nBinsExclFunda = nBins;}
   uint32_t GetBinsExclHarm(){return m_nBinsExclHarm;}
   void SetBinsExclHarm(uint32_t nBins){m_nBinsExclHarm = nBins; }
   //uint32_t GetHarmExclSFDR(){return m_nHarmsExclSFDR;}
   //void SetHarmExclSFDR(uint32_t nBins){m_nHarmsExclSFDR = nBins;}
   SigMtrxScaleUnits_t GetdBScale(){return m_dBScale;}
   void SetdBScale(SigMtrxScaleUnits_t s){m_dBScale = (s == dBc)?s:dBFS;}
   uint32_t GetnSetsAvg(){return m_nSetsAvg;}
   void SetnSetsAvg(uint32_t n){m_nSetsAvg = n;}
   uint32_t GetnBitsSamp(){return m_nBitsSample;}
   void SetnBitsSamp(uint32_t n){m_nBitsSample = n;}
   void SetNoiseLvl(double d){m_noiseRange = d;}
   double GetNoiseLvl(){return m_noiseRange;}



protected slots:
   virtual void OnBeforeDeleteOwner(DataClass *dc);

private slots:
   void OnBeforeDeleteInput(DataClass* dc);
   void OnInputDataSetNameChanged(DataClass* dc);
   void OnNewData(DataClass* source);
   void OnPropertiesViewClosed();

signals:
   void ProcUpdated();

private:
   void processorUpdated();
   void performCalc();
   void calculateMetrics();
   double lin2Db(double val, double scaleVal);
   void copyLinear2dB(const DataSet *dsIn, DataSet *dsOut, SigMtrxScaleUnits_t scale);
   void updateBitsPerSampleFromDataSet();
   void updateSampleRateFromDataSet();

   QMutex m_mutex;


   // FFT processor
   DisplayFFT *m_fft = NULL;
   DataSet* m_dsIn     = NULL;
   DataSet* m_dsMtrx   = NULL;
   DataSet* m_dsFFTHz  = NULL;
   DataSet* m_dsFFTOut = NULL;
   SigMtrxScaleUnits_t m_dBScaleFFT = dBc;
   bool m_autoUpdateSamplingRate = false;
   double m_samplingRateHz = 100000000;
   bool m_adjustWindowToInputSize = true;

   // Metrics
   FrequencySignalMetrics *m_sigMetrx = NULL;
   bool m_autoUpdateBitsPerSamp  = false;
   uint32_t m_nBitsSample        = 12;
   SigMtrxScaleUnits_t m_dBScale = dBc;
   uint32_t m_maxHarmonics   = 6;
   uint32_t m_nBinsExclDC    = 1;
   uint32_t m_nBinsExclFunda = 1;
   uint32_t m_nBinsExclHarm  = 1;
   uint32_t m_nSetsAvg       = 0;
   //uint32_t m_nHarmsExclSFDR = 6;
   double   m_noiseRange     = 9;
};



}// terbit
