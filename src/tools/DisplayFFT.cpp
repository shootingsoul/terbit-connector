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
#include "Tools.h"
#include "DisplayFFT.h"
#include "SignalTools.h"
#include <math.h>

namespace terbit
{

DisplayFFT::DisplayFFT() : m_N(0), m_inputLen(0), m_freqN(0), m_outputType(OUTPUT_MAGNITUDE_DECIBEL), m_cfg(NULL), m_in(NULL), m_inputTemp(NULL), m_out(NULL), m_samplingRate(0),
   m_windowType(WINDOW_HANNING), m_window(NULL), m_windowLen(0), m_windowOption(0), m_removeDC(true)
{
}

DisplayFFT::~DisplayFFT()
{
   delete [] m_inputTemp;
   delete [] m_window;
   free(m_cfg);
   free(m_in);
   free(m_out);
}

bool DisplayFFT::SetInputLen(size_t inputLen, bool adjustWindowLen)
{
   bool res;

   m_inputLen = inputLen;

   if (adjustWindowLen)
   {
      res = SetWindow(m_windowType,inputLen,m_windowOption);
   }
   else
   {
      res = UpdateBuffers();
   }

   if (!res)
   {
      m_inputLen = 0;
   }

   return res;
}

bool DisplayFFT::SetWindow(DisplayFFT::WindowType window, size_t windowLen, double option)
{
   delete [] m_window;
   m_window = NULL;

   m_windowType = window;
   m_windowLen = windowLen;
   m_windowOption = option;

   if (m_windowLen > 0 && m_windowType != WINDOW_NONE)
   {
      m_window = new double[windowLen];

      switch (m_windowType)
      {
      case WINDOW_BOXCAR:
         SignalTools::BoxcarWindow(m_window,m_windowLen);
         break;
      case WINDOW_GAUSSIAN:
         SignalTools::GaussianWindow(m_window,m_windowLen,m_windowOption);
         break;
      case WINDOW_HAMMING:
         SignalTools::HammingWindow(m_window,m_windowLen);
         break;
      case WINDOW_HANNING:
         SignalTools::HanningWindow(m_window,m_windowLen);
         break;
      case WINDOW_TRIANGLE:
         SignalTools::TriangleWindow(m_window,m_windowLen);
         break;
      case WINDOW_TUKEY:
         SignalTools::TukeyWindow(m_window,m_windowLen,m_windowOption);
         break;
      }
   }

   return UpdateBuffers();
}

bool DisplayFFT::UpdateBuffers()
{
   bool res = false;

   delete [] m_inputTemp;
   m_inputTemp = NULL;

   if (m_window == NULL || m_windowLen == m_inputLen)
   {
      m_N = m_inputLen;
   }
   else
   {
      m_N = m_inputLen + m_windowLen - 1;
      m_inputTemp = new double[m_inputLen];
   }

   //N must be power of 2
   size_t tempN = pow(2,floor(log10(m_N)/(double)log10(2)));
   if (m_N != tempN)
   {
      LogWarning(g_logTools.data, QObject::tr("DisplayFFT N must be power of 2.  Truncating input from %1 to %2").arg(m_N).arg(tempN));
      m_N = tempN;
   }

   //release existing (if any)
   free(m_cfg);
   free(m_in);
   free(m_out);

   m_freqN = m_N/2+1;
   m_cfg = kiss_fftr_alloc(m_N, 0/*is_inverse_fft*/, NULL, NULL);
   m_in = (kiss_fft_scalar*)malloc(m_N*sizeof(kiss_fft_scalar));
   m_out = (kiss_fft_cpx*)malloc(m_N*sizeof(kiss_fft_cpx));

   if (m_cfg != NULL && m_in != NULL && m_out != NULL)
   {
      res = true;
   }
   else
   {
      //out of memory?
      LogError(g_logTools.data, QObject::tr("DisplayFFT allocate memory error."));
   }

   return res;
}


size_t DisplayFFT::FastN(size_t N)
{
   return kiss_fftr_next_fast_size_real(N);
}

void DisplayFFT::SetSamplingRate(double samplingRate)
{
   m_samplingRate = samplingRate;
}

template<typename DataType>
void CalcOutput(DisplayFFT::OutputType type, size_t N, kiss_fft_cpx*out, DataType* output, double samplingRate)
{
   DataType* dataOutput;
   size_t i;
   kiss_fft_cpx* dataCpx;
   size_t N2 = N/2+1;

   //calculate final result from complex buffer
   switch (type)
   {
   case DisplayFFT::OUTPUT_MAGNITUDE_LINEAR:
      //y=2*abs(fft(data))/dataLength
      for(i=0, dataCpx = out, dataOutput = output ; i < N2; ++i, ++dataOutput, ++dataCpx)
      {
         //abs of complex value = sqrt(r^2 + i^2) also scale properly
         *dataOutput = (DataType)(2.0/N*sqrt(dataCpx->r*dataCpx->r+dataCpx->i*dataCpx->i));
      }
      break;
   case DisplayFFT::OUTPUT_MAGNITUDE_DECIBEL:
      //y=20*log10(2*abs(fft(data))/N)
      for(i=0, dataCpx = out, dataOutput = output ; i < N2; ++i, ++dataOutput, ++dataCpx)
      {
         //abs of complex value = sqrt(r^2 + i^2) also scale properly
         *dataOutput = (DataType)(20*log10(2.0/N*sqrt(dataCpx->r*dataCpx->r+dataCpx->i*dataCpx->i)));
      }
      break;
   case DisplayFFT::OUTPUT_POWER_SPECTRAL_DENSITY:
      //y=10*log10(abs(fft(data))^2/(N*SamplingRate))
      if (samplingRate == 0)
      {
         LogError(g_logTools.data, QObject::tr("DisplayFFT OUTPUT_POWER_SPECTRAL_DENSITY requires sampling rate that's not 0."));
      }
      else
      {
         for(i=0, dataCpx = out, dataOutput = output ; i < N2; ++i, ++dataOutput, ++dataCpx)
         {
            //abs of complex value = sqrt(r^2 + i^2)
            //we want that squared so we get (r^2 + i^2) then scale properly
            *dataOutput = (DataType)(10*log10((dataCpx->r*dataCpx->r+dataCpx->i*dataCpx->i)/(N*samplingRate)));
         }
      }
      break;
   default:
      LogError(g_logTools.data, QObject::tr("Display FFT unknown output type: %1").arg(type));
   };
}

template<typename DataType>
void DisplayFFT::CalcFFT(const DataType* input)
{   
   size_t i, len;
   double* dataScalar;
   const DataType* dataInput;

   kiss_fft_scalar* in;
   if (m_window && m_windowLen != m_inputLen)
   {
      in = m_inputTemp;
      len = m_inputLen;
   }
   else
   {
      in = m_in;
      len = m_N;
   }

   //copy input data to buffer
   double mean = 0;
   for(i=0, dataScalar = in, dataInput = input ; i < len; ++i, ++dataInput, ++dataScalar)
   {
      *dataScalar = (double)*dataInput;
      mean += *dataScalar;
   }
   mean /= len;
   if (m_removeDC)
   {
      for(i=0, dataScalar = in; i<len; ++i, ++dataScalar)
      {
         *dataScalar = *dataScalar - mean;
      }
   }
   if (m_window)
   {
      //will convolve in-place or from temp-buffer . . . in-place ok when sizes are the same
      SignalTools::Convolve(in,len,m_window,m_windowLen,m_in,m_N);
   }

   kiss_fftr(m_cfg, m_in, m_out);
}

void DisplayFFT::FFT(const float *input, float *output)
{
   CalcFFT<float>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int8_t *input, float *output)
{
   CalcFFT<int8_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint8_t *input, float *output)
{
   CalcFFT<uint8_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int16_t *input, float *output)
{
   CalcFFT<int16_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint16_t *input, float *output)
{
   CalcFFT<uint16_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int32_t *input, float *output)
{
   CalcFFT<int32_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint32_t *input, float *output)
{
   CalcFFT<uint32_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int64_t *input, float *output)
{
   CalcFFT<int64_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint64_t *input, float *output)
{
   CalcFFT<uint64_t>(input);
   CalcOutput<float>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int8_t *input, double *output)
{
   CalcFFT<int8_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint8_t *input, double *output)
{
   CalcFFT<uint8_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int16_t *input, double *output)
{
   CalcFFT<int16_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint16_t *input, double *output)
{
   CalcFFT<uint16_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int32_t *input, double *output)
{
   CalcFFT<int32_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint32_t *input, double *output)
{
   CalcFFT<uint32_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const int64_t *input, double *output)
{
   CalcFFT<int64_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const uint64_t *input, double *output)
{
   CalcFFT<uint64_t>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const float *input, double *output)
{
   CalcFFT<float>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}

void DisplayFFT::FFT(const double *input, double *output)
{
   CalcFFT<double>(input);
   CalcOutput<double>(m_outputType,m_N, m_out,output, m_samplingRate);
}


}
