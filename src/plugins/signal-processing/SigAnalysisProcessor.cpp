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
#include <QtConcurrent/QtConcurrent>
#include "SigAnalysisProcessor.h"
#include "SignalAnalysisView.h"
#include "tools/DisplayFFT.h"
#include "tools/FrequencySignalMetrics.h"
#include "connector-core/Workspace.h"
#include "connector-core/DataSet.h"
#include "connector-core/LogDL.h"
#include "SigAnalysisProcSW.h"
#include "tools/Script.h"
#include <vector>
#include <QObject>
namespace terbit
{

const BlockIOCategory_t SigAnalysisProcessor::OUTPUT_FFT = 0;

SigAnalysisProcessor::SigAnalysisProcessor()
{
   m_fft = new DisplayFFT();
   m_sigMetrx = new FrequencySignalMetrics();
   m_fft->SetOutputType(DisplayFFT::OUTPUT_MAGNITUDE_LINEAR);
}

SigAnalysisProcessor::~SigAnalysisProcessor()
{
   SetDataSet(NULL);

   if (m_dsFFTHz)
   {
      GetWorkspace()->DeleteInstance(m_dsFFTHz->GetAutoId());
      m_dsFFTHz = NULL;
   }
   if (m_dsFFTOut)
   {
      GetWorkspace()->DeleteInstance(m_dsFFTOut->GetAutoId());
      m_dsFFTOut = NULL;
   }
   if (m_dsMtrx)
   {
      GetWorkspace()->DeleteInstance(m_dsMtrx->GetAutoId());
      m_dsMtrx = NULL;
   }

   delete m_fft;
   delete m_sigMetrx;

   ClosePropertiesView();
}

bool SigAnalysisProcessor::ShowPropertiesView()
{
   bool retVal = true;
   SignalAnalysisView *view = new SignalAnalysisView(this);
   GetWorkspace()->AddDockWidget(view);

   return retVal;
}

void SigAnalysisProcessor::ClosePropertiesView()
{
   GetWorkspace()->RemDataClassDocks(this);
}

QString SigAnalysisProcessor::BuildPropertiesViewName()
{
   return QString(tr("%1 Options").arg(GetName()));
}

bool SigAnalysisProcessor::Init()
{
   bool res = true;

   m_dsFFTOut = GetWorkspace()->CreateDataSet(this);
   m_dsFFTOut->SetDisplayViewTypeName(TERBIT_TYPE_XYPLOT);
   if(m_dsIn)
   {
      m_dsFFTOut->SetName(tr("FFT (%1)").arg(m_dsIn->GetName()));
   }
   else
   {
      m_dsFFTOut->SetName(tr("FFT"));
   }

   m_dsMtrx = GetWorkspace()->CreateDataSet(this, false);

   //hack so index buf shows properly, set the owner as the main buf
   m_dsFFTHz = GetWorkspace()->CreateDataSet(m_dsFFTOut);
   m_dsFFTHz->SetDisplayViewTypeName(TERBIT_TYPE_XYPLOT);
   m_dsFFTHz->SetName(tr("Hz"));

   m_dsFFTOut->SetIndexDataSet(m_dsFFTHz);

   AddOutput(OUTPUT_FFT, m_dsFFTOut);

   return res;
}

bool SigAnalysisProcessor::InteractiveInit()
{
   bool retVal = true;
   ShowPropertiesView();
   m_dsFFTOut->ShowDisplayView();
   return retVal;
}

void SigAnalysisProcessor::SetName(const QString &name)
{
   Block::SetName(name);
}

void SigAnalysisProcessor::SetSamplingRate(double samplingRate)
{
  if (samplingRate != m_samplingRateHz)
  {
      m_samplingRateHz = samplingRate;
      UpdateFrequencyXValues();
      processorUpdated();
  }
}

void SigAnalysisProcessor::SetAutoUpdateSamplingRate(bool en)
{
   m_autoUpdateSamplingRate = en;
   //get an inital value from buf
   if (m_autoUpdateSamplingRate && m_dsIn)
   {
      updateSampleRateFromDataSet();
   }
   processorUpdated();
}

void SigAnalysisProcessor::UpdateFrequencyXValues()
{
   DataSet* buf = m_dsFFTHz;
   size_t i, freqs;
   freqs = m_dsFFTHz->GetCount();
   if (freqs > 0)
   {
      double* freqBuf = (double*)buf->GetBufferAddress();
      double freqScale = m_samplingRateHz/2.0/freqs;
      for(i=0; i<freqs; ++i, ++freqBuf)
      {
         *freqBuf = i*freqScale;
      }
      m_dsFFTHz->SetHasData(true);
      emit m_dsFFTHz->NewData(m_dsFFTHz);
   }
}

void SigAnalysisProcessor::processorUpdated()
{
   emit ProcUpdated();
}

void SigAnalysisProcessor::ApplyInputDataClass(DataClass* dc)
{
   if (dc && dc->IsDataSet())
   {
      Block::ApplyInputDataClass(dc);
      DataSet* buf = static_cast<DataSet*>(dc);

      SetDataSet(buf);
      Refresh();
   }
}

void SigAnalysisProcessor::updateSampleRateFromDataSet()
{
   QVariant value = m_dsIn->GetPropertyValue(TERBIT_DATA_PROPERTY_SAMPLING_RATE);
   if (value.isValid())
   {
      if (value.canConvert(QVariant::Double))
      {
         SetSamplingRate(value.toDouble());
      }
      else
      {
         LogWarning2(GetType()->GetLogCategory(),GetName(), tr("Sampling Rate could not be converted to an double."));
      }
   }
}


void SigAnalysisProcessor::updateBitsPerSampleFromDataSet()
{
   QVariant value = m_dsIn->GetPropertyValue(TERBIT_DATA_PROPERTY_SAMPLING_BITS);
   if (value.isValid())
   {
      if (value.canConvert(QVariant::UInt))
      {
         SetnBitsSamp(value.toUInt());
      }
      else
      {
         LogWarning2(GetType()->GetLogCategory(),GetName(), tr("Sampling bits could not be converted to an integer."));
      }
   }
}

void SigAnalysisProcessor::SetDataSet(DataSet *buf)
{
   //wait for background processing before we switch/clear data set
   //stop listening for signals right away . . .
   if (m_dsIn)
   {
      disconnect(m_dsIn,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeDeleteInput(DataClass*)));
      disconnect(m_dsIn,SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
      disconnect(m_dsIn,SIGNAL(NameChanged(DataClass*)),this,SLOT(OnInputDataSetNameChanged(DataClass*)));
   }
   m_mutex.lock();
   m_dsIn = buf;
   if (m_dsIn)
   {
      connect(m_dsIn,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeDeleteInput(DataClass*)));
      //TODO: future have event/signal sync options
      //for now only connect to signal if it's your owner . . .
      if (GetOwner() == m_dsIn)
      {
         connect(m_dsIn,SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
      }
      connect(m_dsIn,SIGNAL(NameChanged(DataClass*)),this,SLOT(OnInputDataSetNameChanged(DataClass*)));

      if (m_autoUpdateSamplingRate)
      {
         updateSampleRateFromDataSet();
      }

      if (m_autoUpdateBitsPerSamp)
      {
         updateBitsPerSampleFromDataSet();
      }

      OnInputDataSetNameChanged(m_dsIn);
      UpdateBuffers(); //update buffers now so any remotes will be accurate or at least have a chance
   }
   m_mutex.unlock();

}

void SigAnalysisProcessor::OnBeforeDeleteInput(DataClass *dc)
{
   if (m_dsIn == dc)
   {
      //our input source is being removed
      SetDataSet(NULL);
   }
}

void SigAnalysisProcessor::OnBeforeDeleteOwner(DataClass *dc)
{
   Block::OnBeforeDeleteOwner(dc);
   //remove ourself if they owned us
   GetWorkspace()->DeleteInstance(this->GetAutoId());
}

void SigAnalysisProcessor::OnNewData(DataClass* source)
{
   source;
   Refresh();
}

void SigAnalysisProcessor::OnInputDataSetNameChanged(DataClass *dc)
{
   dc;
   QString name = QString(tr("Signal Analysis (%1)").arg(m_dsIn->GetName()));
   SetName(name);
   m_dsFFTOut->SetName(tr("FFT (%1)").arg(m_dsIn->GetName()));
}

void SigAnalysisProcessor::OnPropertiesViewClosed()
{
   ;
}

#if 0
void SigAnalysisProcessor::copyLinear2dB(const DataSet* dsIn, DataSet* dsOut, SigMtrxScaleUnits_t scale)
{
   if(dsIn->GetCount() > dsOut->GetCount())
   {
      LogError2(GetType()->GetLogCategory(), GetName(), tr("The destination FFT buffer is not large enough to hold converted data from source FFT buffer"));
   }
   else
   {
      double scaleVal = (1 << m_nBitsSample)-1;
      // if dBc, then find max value for scaling

      if(scale != dBFS)
      {
         scaleVal = dsIn->GetValueAtIndex(0);
         for(uint32_t i = 0; i < dsIn->GetCount(); ++i)
         {
            if(scaleVal < dsIn->GetValueAtIndex(i))
            {
               scaleVal = dsIn->GetValueAtIndex(i);
            }
         }
      }

      for(size_t i = 0; i < dsIn->GetCount(); ++i)
      {
         dsOut->SetValueAtIndex(i, lin2Db(dsIn->GetValueAtIndex(i), scaleVal));
      }
   }
}

double SigAnalysisProcessor::lin2Db(double val, double scaleVal)
{
   return 20 * log(val/scaleVal);
}

#endif
void SigAnalysisProcessor::Refresh()
{
   if(m_dsIn)
   {
      QtConcurrent::run(this, &SigAnalysisProcessor::performCalc);
   }
}

void SigAnalysisProcessor::calculateMetrics()
{
   if (m_dsMtrx)
   {
      DataSet& b = *m_dsMtrx;
      if (b.GetHasData() && b.GetCount() > 0)
      {
         if (b.GetDataType() == TERBIT_DOUBLE && b.GetStrideBytes() == sizeof(double))
         {
            m_sigMetrx->Calculate((double*)b.GetBufferAddress(),(double*)m_dsFFTOut->GetBufferAddress(), b.GetCount(),m_maxHarmonics, m_nBinsExclDC,m_nBinsExclFunda,m_nBinsExclHarm, (m_dBScale == dBFS), m_nBitsSample, m_noiseRange);

            // m_dsMtrx has FFT output  Convert for display
            // copyLinear2dB(m_dsMtrx, m_dsFFTOut, m_dBScaleFFT);
            m_dsFFTOut->SetHasData(true);
            emit m_dsFFTOut->NewData(m_dsFFTOut);
         }
         else
         {
            LogError2(GetType()->GetLogCategory(), GetName(), tr("The frequency metrics display requires contiguous data of the double data type.  The input data set is not compatible.  Input data set: %1").arg(m_dsIn->GetName()));
         }
      }
   }
}



bool SigAnalysisProcessor::UpdateBuffers()
{
   bool res = true;
   //crop len to largest power of two
   size_t len = m_dsIn->GetCount();

   if (len != m_fft->GetInputLen())
   {
      if (m_fft->SetInputLen(len, m_adjustWindowToInputSize))
      {
         size_t freqN = m_fft->GetFrequencyN();
         m_dsFFTOut->CreateBuffer(TERBIT_DOUBLE, 0, freqN);
         m_dsFFTHz->CreateBuffer(TERBIT_DOUBLE,0, freqN);
         m_dsMtrx->CreateBuffer(TERBIT_DOUBLE, 0, freqN);
         UpdateFrequencyXValues();         
      }
      else
      {
         res = false;
      }
   }
   return res;
}

bool SigAnalysisProcessor::SetAdjustWindowToInputSize(bool value)
{
   bool retVal = true;
   m_adjustWindowToInputSize = value;
   if (value)
   {
      m_mutex.lock();
      m_fft->SetWindow(m_fft->GetWindowType(), 0, m_fft->GetWindowOption());
      m_mutex.unlock();
      //Calculate();
   }
   return retVal;
}

void SigAnalysisProcessor::SetWindow(DisplayFFT::WindowType type, size_t len, double option)
{
   if (m_adjustWindowToInputSize)
   {
      len = 0;//m_fft->GetInputLen();
   }
   m_mutex.lock();
   if (m_fft->SetWindow(type,len,option) == false)
   {
      LogError2(GetType()->GetLogCategory(), GetName(),tr("Set Window Failed"));
   }
   m_mutex.unlock();
   //Calculate();
}

double SigAnalysisProcessor::GetFundaFreq()
{
   double retVal = 0;
   if(0 != m_sigMetrx->GetHarmonics().size())
   {
      uint32_t idx = m_sigMetrx->GetHarmonics()[0]->GetIndex();
      if(idx < m_dsFFTHz->GetCount())
      {
         retVal =  m_dsFFTHz->GetValueAtIndex(idx);
      }
   }
   return retVal;
}

uint32_t SigAnalysisProcessor::GetFundaBin()
{
   double retVal = 0;
   if(0 != m_sigMetrx->GetHarmonics().size())
   {
      retVal = m_sigMetrx->GetHarmonics()[0]->GetIndex();
   }
   return retVal;

}

double SigAnalysisProcessor::GetFundaAmp()
{
   return 0 - m_sigMetrx->GetFundamentalDecibels();
}

size_t SigAnalysisProcessor::GetHarmIndex(uint32_t harm)
{
   size_t retVal = 0;
   if (m_sigMetrx->GetHarmonics().size() > harm)
   {
      retVal = m_sigMetrx->GetHarmonics().at(harm)->GetIndex();
   }
   return retVal;
}

double SigAnalysisProcessor::GetHarmAmp(uint32_t harm)
{
   double retVal = 0;
   if (m_sigMetrx->GetHarmonics().size() > harm)
   {
      retVal = m_sigMetrx->GetHarmonics().at(harm)->GetAmplitude();
   }
   return retVal;
}

double SigAnalysisProcessor::GetHarmFreq(uint32_t harm)
{
   double retVal = 0;
   if (m_sigMetrx->GetHarmonics().size() > harm)
   {
      uint32_t idx = m_sigMetrx->GetHarmonics().at(harm)->GetIndex();
      if(idx < m_dsFFTHz->GetCount())
      {
         retVal =  m_dsFFTHz->GetValueAtIndex(idx);
      }
   }
   return retVal;
}




void SigAnalysisProcessor::performCalc()
{
   if (m_dsIn && m_dsIn->GetHasData()) //paranoid check
   {
      m_mutex.lock();
      try
      {
         //ensure we have latest sampling rate
         if (m_autoUpdateSamplingRate)
         {
            updateSampleRateFromDataSet();
         }
         if(m_autoUpdateBitsPerSamp)
         {
            updateBitsPerSampleFromDataSet();
         }
         m_fft->SetSamplingRate(m_samplingRateHz);
         if (UpdateBuffers())
         {
            switch (m_dsIn->GetDataType())
            {
            case TERBIT_DOUBLE:
               m_fft->FFT((double*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_FLOAT:
               m_fft->FFT((float*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_INT8:
               m_fft->FFT((int8_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_UINT8:
               m_fft->FFT((uint8_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_INT16:
               m_fft->FFT((int16_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_UINT16:
               m_fft->FFT((uint16_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_INT32:
               m_fft->FFT((int32_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_UINT32:
               m_fft->FFT((uint32_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_INT64:
               m_fft->FFT((int64_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            case TERBIT_UINT64:
               m_fft->FFT((uint64_t*)m_dsIn->GetBufferAddress(),(double*)m_dsMtrx->GetBufferAddress());
               break;
            };
            m_dsMtrx->SetHasData(true);
            // Perform metrics
            calculateMetrics();
            processorUpdated();
         }
      }
      catch(...)
      {
         LogError2(GetType()->GetLogCategory(),GetName(),tr("An exception occured while calculating the FFT."));
      }
      m_mutex.unlock();
   }
   else
   {
      LogWarning2(GetType()->GetLogCategory(),GetName(),tr("The FFT calculate was called, but the input dataset has no data."));
   }

}



QObject *SigAnalysisProcessor::CreateScriptWrapper(QJSEngine *se)
{
   return new SigAnalysisProcSW(se, this);
}


void SigAnalysisProcessor::BuildRestoreScript(ScriptBuilder &script, const QString &variableName)
{
#if 1

   if (m_dsIn)
   {
      script.add(QString("%1.SetDataSet(%2);").arg(variableName).arg(ScriptEncode(m_dsIn->GetUniqueId())));
   }

   script.add(QString("%1.SetdBScale(%1.%2);").arg(variableName).arg((GetdBScale() == dBc ? "DBC" : "DBFS")));

   QString win;
   switch (GetWindowType())
   {
   case DisplayFFT::WINDOW_NONE:
      win = "WINDOW_NONE";
      break;
   case DisplayFFT::WINDOW_BOXCAR:
      win = "WINDOW_BOXCAR";
      break;
   case DisplayFFT::WINDOW_GAUSSIAN:
      win = "WINDOW_GAUSSIAN";
      break;
   case DisplayFFT::WINDOW_HAMMING:
      win = "WINDOW_HAMMING";
      break;
   case DisplayFFT::WINDOW_HANNING:
      win = "WINDOW_HANNING";
      break;
   case DisplayFFT::WINDOW_TRIANGLE:
      win = "WINDOW_TRIANGLE";
      break;
   case DisplayFFT::WINDOW_TUKEY:
      win = "WINDOW_TUKEY";
      break;
   }

   // FFT
   script.add(QString("%1.UpdateSampleRateFromDataSet(%2);").arg(variableName).arg(QString::number(GetAutoUpdateSamplingRate()?1:0)));
   script.add(QString("%1.SetSamplingRate(%2);").arg(variableName).arg(QString::number(GetSamplingRate())));
   script.add(QString("%1.SetRemoveDC(%2);").arg(variableName).arg(QString::number(GetRemoveDC()?1:0)));
   script.add(QString("%1.SetAdjustWindowToInputSize(%2);").arg(variableName).arg(QString::number(GetAdjustWindowToInputSize()?1:0)));
   script.add(QString("%1.SetWindow(%1.%2, %3, %4);").arg(variableName).arg(win).arg(QString::number(GetWindowLen())).arg(QString::number(GetWindowOption())));


   // Metrics
   script.add(QString("%1.UpdateBitsPerSampleFromDataSet(%2);").arg(variableName).arg(QString::number(GetUpdateBitsPerSampleFromDataSet()?1:0)));
   script.add(QString("%1.SetMaxHarmonics(%2);").arg(variableName).arg(QString::number(GetMaxHarmonics())));
   script.add(QString("%1.SetBinsExclDC(%2);").arg(variableName).arg(QString::number(GetBinsExclDC())));
   script.add(QString("%1.SetBinsExclFunda(%2);").arg(variableName).arg(QString::number(GetBinsExclFunda())));
   script.add(QString("%1.SetBinsExclHarm(%2);").arg(variableName).arg(QString::number(GetBinsExclHarm())));
   //script.add(QString("%1.SetHarmExclSFDR(%2);").arg(variableName).arg(QString::number(GetHarmExclSFDR())));
   script.add(QString("%1.SetnBitsSamp(%2);").arg(variableName).arg(QString::number(GetnBitsSamp())));

//   if (m_view)
   {
      script.add(QString("%1.ShowPropertiesWindow();").arg(variableName));
   }
#endif
}



}// terbit
