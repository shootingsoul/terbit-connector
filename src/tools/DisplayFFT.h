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

#include <stdint.h>
#include "kiss_fftr.h"

namespace terbit
{

/*!
 * \brief Calculate FFT on data for display
 *
 *  N must be even.  Use FastN function to get a valid N size that will perform well.
 *  Only return positive half of the spectrum (Frequency N = N/2+1)
 *
 *
 *  Linear Magnitude output
 *  --------------------------------------------------------------------------------------------------
 *  Interested in displaying the magnitude of complex result from an FFT "properly" scaled (scaling can be open to interpretation)
 *  y=2*abs(fft(data))/N
 *
 *
 *  Decibel Magnitude output
 *  --------------------------------------------------------------------------------------------------
 *  Display the magnitude of complex result from an FFT converted to a decibal scale
 *  Convert linear magnitude to decibels
 *  y=20*log10(2*abs(fft(data))/N)
 *
 *
 *  Power Spectral Density output
 *  --------------------------------------------------------------------------------------------------
 *  y=10*log10(abs(fft(data))^2/(N*SamplingRate))
 *
 */
class DisplayFFT
{
public:
   DisplayFFT();
   ~DisplayFFT();

   enum OutputType
   {
      OUTPUT_MAGNITUDE_LINEAR = 0,
      OUTPUT_MAGNITUDE_DECIBEL,
      OUTPUT_POWER_SPECTRAL_DENSITY
   };

   enum WindowType
   {
      WINDOW_NONE = 0,
      WINDOW_BOXCAR,
      WINDOW_TRIANGLE,
      WINDOW_GAUSSIAN,
      WINDOW_TUKEY,
      WINDOW_HAMMING,
      WINDOW_HANNING
   };

   size_t GetInputLen() { return m_inputLen; }
   size_t GetN() { return m_N; }
   size_t GetFrequencyN() { return m_freqN; }
   bool SetInputLen(size_t inputLen, bool adjustWindowLen);
   static size_t FastN(size_t N);
   void SetSamplingRate(double samplingRate);
   double GetSamplingRate() { return m_samplingRate; }

   WindowType GetWindowType() { return m_windowType; }
   double GetWindowOption() { return m_windowOption; }
   size_t GetWindowLen() { return m_windowLen; }
   bool SetWindow(WindowType window, size_t windowLen, double option);

   bool GetRemoveDC() { return m_removeDC; }
   void SetRemoveDC(bool value) { m_removeDC = value; }

   OutputType GetOutputType() { return m_outputType; }
   void SetOutputType(OutputType outputType) { m_outputType = outputType; }

   void FFT(const int8_t* input, float* output);
   void FFT(const uint8_t* input, float* output);
   void FFT(const int16_t* input, float* output);
   void FFT(const uint16_t* input, float* output);
   void FFT(const int32_t* input, float* output);
   void FFT(const uint32_t* input, float* output);
   void FFT(const int64_t* input, float* output);
   void FFT(const uint64_t* input, float* output);
   void FFT(const float* input, float* output);

   void FFT(const int8_t* input, double* output);
   void FFT(const uint8_t* input, double* output);
   void FFT(const int16_t* input, double* output);
   void FFT(const uint16_t* input, double* output);
   void FFT(const int32_t* input, double* output);
   void FFT(const uint32_t* input, double* output);
   void FFT(const int64_t* input, double* output);
   void FFT(const uint64_t* input, double* output);
   void FFT(const float* input, double* output);
   void FFT(const double* input, double* output);

private:
   template<typename DataType>
   void CalcFFT(const DataType* input);
   bool UpdateBuffers();

   size_t m_N, m_inputLen;
   size_t m_freqN;
   OutputType m_outputType;
   kiss_fftr_cfg m_cfg;
   kiss_fft_scalar* m_in, *m_inputTemp;
   kiss_fft_cpx* m_out;
   double m_samplingRate;
   WindowType m_windowType;
   double *m_window;
   size_t m_windowLen;
   double m_windowOption;
   bool m_removeDC;
};

}
