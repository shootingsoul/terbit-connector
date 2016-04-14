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
#include "SigAnalysisProcessor.h"
#include "connector-core/Block.h"

QT_BEGIN_INCLUDE_NAMESPACE
class QJSEngine;
QT_END_INCLUDE_NAMESPACE


namespace terbit
{

ScriptDocumentation *BuildScriptDocumentationSigAnalysisProc();

class SigAnalysisProcSW : public BlockSW
{
   Q_OBJECT
public:
   SigAnalysisProcSW(QJSEngine *se, SigAnalysisProcessor *sap);
   ~SigAnalysisProcSW(){}

   Q_PROPERTY(QJSValue DBC READ GetDBC)
   QJSValue GetDBC() { return dBc; }

   Q_PROPERTY(QJSValue DBFS READ GetDBFS)
   QJSValue GetDBFS() { return dBFS; }

   Q_PROPERTY(QJSValue WINDOW_NONE READ GetWINDOW_NONE)
   QJSValue GetWINDOW_NONE() { return DisplayFFT::WINDOW_NONE; }

   Q_PROPERTY(QJSValue WINDOW_BOXCAR READ GetWINDOW_BOXCAR)
   QJSValue GetWINDOW_BOXCAR() { return DisplayFFT::WINDOW_BOXCAR; }

   Q_PROPERTY(QJSValue WINDOW_GAUSSIAN READ GetWINDOW_GAUSSIAN)
   QJSValue GetWINDOW_GAUSSIAN() { return DisplayFFT::WINDOW_GAUSSIAN; }

   Q_PROPERTY(QJSValue WINDOW_HAMMING READ GetWINDOW_HAMMING)
   QJSValue GetWINDOW_HAMMING() { return DisplayFFT::WINDOW_HAMMING; }

   Q_PROPERTY(QJSValue WINDOW_HANNING READ GetWINDOW_HANNING)
   QJSValue GetWINDOW_HANNING() { return DisplayFFT::WINDOW_HANNING; }

   Q_PROPERTY(QJSValue WINDOW_TRIANGLE READ GetWINDOW_TRIANGLE)
   QJSValue GetWINDOW_TRIANGLE() { return DisplayFFT::WINDOW_TRIANGLE; }

   Q_PROPERTY(QJSValue WINDOW_TUKEY READ GetWINDOW_TUKEY)
   QJSValue GetWINDOW_TUKEY() { return DisplayFFT::WINDOW_TUKEY; }

   Q_INVOKABLE void SetdBScale(int n){if(n == dBc || n == dBFS)m_proc->SetdBScale((SigMtrxScaleUnits_t)n);}

   // Metrics
   Q_INVOKABLE void UpdateBitsPerSampleFromDataSet(bool en){m_proc->UpdateBitsPerSampleFromDataSet(en);}
   Q_INVOKABLE void SetMaxHarmonics(unsigned n){m_proc->SetMaxHarmonics(n);}
   Q_INVOKABLE void SetBinsExclDC(unsigned n);
   Q_INVOKABLE void SetBinsExclFunda(unsigned n){m_proc->SetBinsExclFunda(n);}
   Q_INVOKABLE void SetBinsExclHarm(unsigned n){m_proc->SetBinsExclHarm(n);}
   // deactivated Q_INVOKABLE void SetHarmExclSFDR(unsigned n){m_proc->SetHarmExclSFDR(n);}
   // not implemented Q_INVOKABLE void SetnSetsAvg(unsigned n){m_proc->SetnSetsAvg(n);}
   Q_INVOKABLE void SetnBitsSamp(unsigned n){m_proc->SetnBitsSamp(n);}

   // FFT
   Q_INVOKABLE void UpdateSampleRateFromDataSet(bool en){m_proc->UpdateSampleRateFromDataSet(en);}
   Q_INVOKABLE void SetSamplingRate(double n){m_proc->SetSamplingRate(n);}
   Q_INVOKABLE void SetRemoveDC(bool rem){m_proc->SetRemoveDC(rem);}
   Q_INVOKABLE void SetAdjustWindowToInputSize(bool en){m_proc->SetAdjustWindowToInputSize(en);}
   Q_INVOKABLE void SetWindow(int type, double len, double option){m_proc->SetWindow((DisplayFFT::WindowType)type, (size_t)len, option);}


   Q_INVOKABLE void SetDataSet(const QJSValue& valueDS);


   Q_INVOKABLE double GetFundaFreq() { return m_proc->GetFundaFreq(); }
   Q_INVOKABLE int GetFundaBin() { return m_proc->GetFundaBin(); }
   Q_INVOKABLE double GetFundaAmp() { return m_proc->GetFundaAmp(); }
   Q_INVOKABLE int GetHarmonicCount() { return m_proc->GetHarmonicCount(); }
   Q_INVOKABLE double GetHarmIndex(int harm) { return m_proc->GetHarmIndex(harm); }
   Q_INVOKABLE double GetHarmAmp(int harm) { return m_proc->GetHarmAmp(harm); }
   Q_INVOKABLE double GetHarmFreq(int harm) { return m_proc->GetHarmFreq(harm); }

   Q_INVOKABLE double GetSNR(){return m_proc->GetSNR();}
   Q_INVOKABLE double GetTHD(){return m_proc->GetTHD();}
   Q_INVOKABLE double GetSFDR(){return m_proc->GetSFDR();}
   Q_INVOKABLE double GetSINAD(){return m_proc->GetSINAD();}
   Q_INVOKABLE double GetENOB(){return m_proc->GetENOB();}

private:
   SigAnalysisProcessor *m_proc = NULL;
   QJSEngine *m_scriptEngine = NULL;

};

}
