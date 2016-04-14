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
#include "SigAnalysisProcSW.h"
#include "connector-core/DataClass.h"
#include "connector-core/LogDL.h"
#include "connector-core/Workspace.h"
#include "SigAnalysisProcessor.h"

namespace terbit
{


ScriptDocumentation *BuildScriptDocumentationSigAnalysisProc()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("Signal analysis processor."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetDataSet"), "SetDataSet(ds);",QObject::tr("Sets the data set to process.  The ds variable may be a reference or unique id string.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetdBScale"), "SetdBScale(scale);",QObject::tr("The scale is an integer to represent dBc (0) or dBFS (1).")));

   d->AddScriptlet(new Scriptlet(QObject::tr("UpdateBitsPerSampleFromDataSet"), "UpdateBitsPerSampleFromDataSet(value);",QObject::tr("Boolean option to use the bits per sample from the input data set property.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetnBitsSamp"), "SetnBitsSamp(bits);",QObject::tr("Manually set the number of bits for the input signal.  This is needed to determine dBFS.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetMaxHarmonics"), "SetMaxHarmonics(max);",QObject::tr("The maximum number of harmonics to use when calculating metrics.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetBinsExclDC"), "SetBinsExclDC(bins);",QObject::tr("The number of bins to exclude around the DC when calculating metrics.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetBinsExclFunda"), "SetBinsExclFunda(bins);",QObject::tr("The number of bins to exclude around the fundamental frequency when calculating metrics.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetBinsExclHarm"), "SetBinsExclHarm(bins);",QObject::tr("The number of bins to exclude around the harmonics (NOT including the fundamental) when calculating metrics.")));

   // FFT
   d->AddScriptlet(new Scriptlet(QObject::tr("UpdateSampleRateFromDataSet"), "UpdateSampleRateFromDataSet(value);",QObject::tr("Boolean option to use the sampling rate from the input data set property.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetSamplingRate"), "SetSamplingRate(rate);",QObject::tr("Manually set the sampling rate.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetRemoveDC"), "SetRemoveDC(dc);",QObject::tr("Remove the DC before calculating the FFT.  This removes the mean of the signal.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetAdjustWindowToInputSize"), "SetAdjustWindowToInputSize(value);",QObject::tr("When using windowing, this boolean option automatically adjusts the window size to the input signal size.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetWindow"), "SetWindow(windowType, windowLength, option);",QObject::tr("Set the window settings to use.  Use the window type enum.  A length of 0 will adjust the window size to the input size, otherwise specify a fixed window size to convolve with the input signal.  The option applies for window types gaussian (alpha value) and tukey (r value)")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetFundaFreq"), "GetFundaFreq();",QObject::tr("Returns the fundamental frequency.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetFundaBin"), "GetFundaBin();",QObject::tr("Returns the fundamental bin.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetFundaAmp"), "GetFundaAmp();",QObject::tr("Returns the fundamental amplitude (always dBFS).")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetHarmonicCount"), "GetHarmonicCount();",QObject::tr("Returns the number of harmonics including the fundamental.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetHarmIndex"), "GetHarmIndex(harm);",QObject::tr("Returns the harmonic index.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetHarmAmp"), "GetHarmAmp(harm);",QObject::tr("Returns the harmonic amplitude (dBc or dBFS).")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetHarmFreq"), "GetHarmFreq(harm);",QObject::tr("Returns the harmonic frequency.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetSNR"), "GetSNR();",QObject::tr("Returns the signal to noise ratio (SNR).")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetTHD"), "GetTHD();",QObject::tr("Returns the total harmonic distortion (THD).")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetSFDR"), "GetSFDR();",QObject::tr("Returns the spurious free dynamic range (SFDR).")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetSINAD"), "GetSINAD();",QObject::tr("Returns the signal to noise and distortion ratio (SINAD).")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetENOB"), "GetENOB();",QObject::tr("Returns the effective number of bits (ENOB).")));

   ScriptDocumentation* scale = new ScriptDocumentation();
   scale->SetName(QObject::tr("dBScale"));
   scale->SetSummary(QObject::tr("Decibel scale"));
   scale->AddScriptlet(new Scriptlet(QObject::tr("dBc"), "DBC",QObject::tr("Carrier scale")));
   scale->AddScriptlet(new Scriptlet(QObject::tr("dBFS"), "DBFS",QObject::tr("Full scale")));
   d->AddSubDocumentation(scale);

   ScriptDocumentation* w = new ScriptDocumentation();
   w->SetName(QObject::tr("Windowing"));
   w->SetSummary(QObject::tr("Windowing functions"));
   w->AddScriptlet(new Scriptlet(QObject::tr("None"), "WINDOW_NONE",QObject::tr("No windowing applied to the input signal.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Boxcar"), "WINDOW_BOXCAR",QObject::tr("Boxcar window.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Gaussian"), "WINDOW_GAUSSIAN",QObject::tr("Gaussian window.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Hamming"), "WINDOW_HAMMING",QObject::tr("Hamming window.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Hanning"), "WINDOW_HANNING",QObject::tr("Hanning window.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Triangle"), "WINDOW_TRIANGLE",QObject::tr("Triangle window.")));
   w->AddScriptlet(new Scriptlet(QObject::tr("Tukey"), "WINDOW_TUKEY",QObject::tr("Tukey window.")));
   d->AddSubDocumentation(w);

   return d;
}

SigAnalysisProcSW::SigAnalysisProcSW(QJSEngine *se, SigAnalysisProcessor *sap) : BlockSW(se, sap), m_scriptEngine(se), m_proc(sap)
{

}

void SigAnalysisProcSW::SetBinsExclDC(unsigned n)
{
   m_proc->SetBinsExclDC(n);
}

void SigAnalysisProcSW::SetDataSet(const QJSValue& valueDS)
{
   DataClass* dc = m_proc->GetWorkspace()->FindInstance(valueDS);
   if (dc && dc->IsDataSet())
   {
      m_proc->SetDataSet(static_cast<DataSet*>(dc));
   }
   else
   {
      LogError2(m_proc->GetType()->GetLogCategory(), m_proc->GetName(),tr("Script Signal Analysis Processor SetDataSet invalid argument"));
   }
}
}


