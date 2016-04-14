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
#include "FrequencyMetricsLibSW.h"
#include "tools/FrequencySignalMetrics.h"
#include "connector-core/Workspace.h"
#include "connector-core/DataSet.h"
#include "connector-core/LogDL.h"

namespace terbit
{

FrequencyMetricsLibSW::FrequencyMetricsLibSW(Workspace *workspace) : QObject(), m_workspace(workspace), m_fftDb(NULL),m_fftDbLen(0), m_fundamental(0)
{
   m_metrics = new FrequencySignalMetrics();
}

FrequencyMetricsLibSW::~FrequencyMetricsLibSW()
{
   delete m_metrics;
   delete [] m_fftDb;
}

void FrequencyMetricsLibSW::Calculate(const QJSValue &dataSet)
{
   DataClass* dc = m_workspace->FindInstance(dataSet);
   if (dc && dc->IsDataSet())
   {
      auto ds = static_cast<DataSet*>(dc);
      if (ds->GetDataType() == TERBIT_DOUBLE && ds->GetStrideBytes() == TerbitDataTypeSize(ds->GetDataType()))
      {
         if (ds->GetCount() > m_fftDbLen)
         {
            delete [] m_fftDb;
            m_fftDbLen = ds->GetCount();
            m_fftDb = new double[m_fftDbLen];
         }

         m_metrics->Calculate((double*)ds->GetBufferAddress(),m_fftDb, ds->GetCount(),m_maxHarmonicCount,m_binsDC,m_binsFundamental,m_binsHarmonics,m_fullScale,m_bits,m_noiseDb);
         auto idx = ds->GetIndexDataSet();
         if (idx)
         {
            if (m_metrics->GetHarmonics().size() > 0)
            {
               m_fundamental = idx->GetValueAtIndex(m_metrics->GetHarmonics()[0]->GetIndex());
            }
            else
            {
               m_fundamental = 0;
            }
         }
         else
         {
            LogWarning(g_log.general,tr("FrequencyMetrics input data set does not have an index data set: %1").arg(ds->GetName()));
         }
      }
      else
      {
         LogError(g_log.general,tr("FrequencyMetrics calculate input data set must be a contiguous double: %1").arg(ds->GetName()));
      }
   }
   else
   {
      LogError(g_log.general,tr("FrequencyMetrics calculate invalid input data set."));
   }
}

double FrequencyMetricsLibSW::GetFundamentalFrequency()
{   
   return m_fundamental;
}

int FrequencyMetricsLibSW::GetHarmonicCount()
{
   return m_metrics->GetHarmonics().size();
}

double FrequencyMetricsLibSW::GetSNR()
{
   return m_metrics->GetSNR();
}

double FrequencyMetricsLibSW::GetTHD()
{
   return m_metrics->GetTHD();
}

double FrequencyMetricsLibSW::GetSFDR()
{
   return m_metrics->GetSFDR();
}

double FrequencyMetricsLibSW::GetSINAD()
{
   return m_metrics->GetSINAD();
}

double FrequencyMetricsLibSW::GetENOB()
{
   return m_metrics->GetENOB();
}

}
