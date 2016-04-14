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
#include "FrequencySignalMetrics.h"

namespace terbit {


FrequencySignalMetrics::FrequencySignalMetrics() :  m_snr(0), m_thd(0), m_sfdr(0), m_sinad(0), m_enob(0), m_noiseFloor(0), m_fundamentalDecibels(0)
{
}

FrequencySignalMetrics::~FrequencySignalMetrics()
{
   ClearHarmonics();
}

void FrequencySignalMetrics::ClearHarmonics()
{
   for(auto it : m_harmonics)
   {
      delete it;
   }
   m_harmonics.clear();
}

FrequencySignalMetrics::HarmonicInfo::HarmonicInfo() : m_index(0), m_startIndex(0), m_endIndex(0), m_amplitude(0)
{
}

void FrequencySignalMetrics::AddHarmonic(size_t index, size_t bins, size_t binsDC, size_t count)
{
   auto harm = new HarmonicInfo();
   harm->m_index = index;
   //don't go into the DC for harmonics
   if (harm->m_index > binsDC + bins)
   {
      harm->m_startIndex = harm->m_index - bins;
   }
   else
   {
      harm->m_startIndex = binsDC;
   }
   harm->m_endIndex = harm->m_index + bins;
   if (harm->m_endIndex >= count)
   {
      harm->m_endIndex = count - 1;
   }
   m_harmonics.push_back(harm);
}

void FrequencySignalMetrics::Calculate(double* fft, double* fftDb, size_t count, int maxHarmonicCount, int binsDC, int binsFundamental, int binsHarmonics, bool fullScale, int bits, double noiseDb)
{
   double *current;
   size_t i;

   ++binsDC; //make binsDC include the DC point itself

   ClearHarmonics();

   //determine fundamental index (max value index)
   //skip DC and last bin
   size_t fundamentalIndex = binsDC;
   double fundamental = *(fft+binsDC);
   current = fft+(binsDC+1);
   for(i=binsDC+1; i<count-1; ++i, ++current)
   {
      if (*current > fundamental)
      {
         fundamental = *current;
         fundamentalIndex = i;
      }
   }

   m_snr = m_thd = m_sfdr = m_sinad = m_enob = 0.0;

   //convert input signal to decibels (dBc or dBFS)
   //input fft signal is linear magnitude scaled properly (i.e. abs(fft(signal))*2/N, where N is original signal length and only includes points to nyquist (N/2+1))
   //dBc is to scale relative to carrier (i.e. fundamental)
   //dbFS is to scale relative to full scale (i.e. max possible value based on number of bits)
   //when the full scale max value is converted to frequency, the real part is zero and imaginary part is all 1's for bits-1
   //thus the full scale value to use in the frequency domain is 2^(bits-1)
   //
   //some of the metrics needs to be offset for dBFS after being calculated for dBc
   //the offset is the full scale dB value minus the fundamental dB value
   //
   double dbScale, dbMetricOffset;
   m_fundamentalDecibels = 20*log10(pow(2,bits-1)) - 20*log10(fundamental);
   if (fullScale)
   {
      dbMetricOffset = m_fundamentalDecibels;
      dbScale = pow(2,bits-1); //scale so fftDb is in dbFS
   }
   else
   {
      dbScale = fundamental;
      dbMetricOffset = 0;
   }


   double* orig = fft;
   current = fftDb;
   for(i=0;i<count;++i, ++current, ++orig)
   {
      *current = 20 * log10(*orig / dbScale);
   }

   //Calculate SFDR and noise floor
   double fftDbMean = 0; //skip dc and last bin
   current = fftDb+binsDC;
   for(i=binsDC;i<count-1;++i, ++current)
   {
      fftDbMean += *current;
   }
   fftDbMean /= (count-binsDC-1);

   double noiseFloor = 0;
   m_sfdr = fftDbMean;
   current = fftDb+binsDC;
   for(i=binsDC;i<count - 1;++i, ++current)
   {
      if ((i < fundamentalIndex - binsFundamental) || (i > fundamentalIndex + binsFundamental))
      {
         if (*current > m_sfdr)
         {
            m_sfdr = *current;
         }
         noiseFloor += *current;
      }
   }
   m_sfdr *= -1; //already offset properly due to scalling of fftDb

   //calculate noise floor and noise top
   noiseFloor += fftDbMean;
   noiseFloor /= (count - binsDC - (2*binsFundamental) - 1); //don't include extra fundamental bins or dc.  The fundamental point cancels adding the mean
   double noiseTop = noiseFloor + noiseDb;

   //calculate harmonics (and fundamental with bins)
   //ensure it's above the noiseTop

   //build up harmonic indicies (including fundamental)
   //verify they are above the noise top
   AddHarmonic(fundamentalIndex,binsFundamental, binsDC, count);
   size_t fullCount = 2* (count+1);
   for(i=2; i<=maxHarmonicCount; ++i)
   {
      size_t idx = fundamentalIndex*i % fullCount;
      if (idx >= count)
      {
         idx = fullCount - idx;
      }
      if (idx < binsDC || idx >= count - 1)
      {
         //don't have harmonic based in DC or at the very end
         idx = count - 2;
      }
      if (fftDb[idx] > noiseTop)
      {
         AddHarmonic(idx,binsHarmonics, binsDC, count);
      }
   }

   size_t noisePoints, harmonicPoints, fundamentalPoints;
   noisePoints = harmonicPoints = fundamentalPoints = 0;
   double noiseSquaredSum, harmonicsSquaredSum, fundamentalSquaredSum;
   noiseSquaredSum = harmonicsSquaredSum = fundamentalSquaredSum = 0.0;

   current = fft+binsDC; //skip DC and last point as well
   double* currentDb = fftDb+binsDC;
   for(i=binsDC; i<count - 1; ++i, ++current, ++currentDb)
   {
      //add to harmonics/fundamental or noise
      double value = *current;
      bool processed = false;
      for(auto harm : m_harmonics)
      {
         if (i >= harm->m_startIndex && i <= harm->m_endIndex)
         {
            processed = true;
            if (harm->m_index == i)
            {
               //only add for index match on harmonic (exclude adjacent bins from metric calculations)
               harm->m_amplitude = *currentDb;
               //add to squared sum totals for other metric calcs
               if (harm->m_index == fundamentalIndex)
               {
                  fundamentalSquaredSum += value*value;
                  ++fundamentalPoints;
               }
               else
               {
                  harmonicsSquaredSum += value*value;
                  ++harmonicPoints;
               }
            }
            break;
         }
      }
      if (!processed)
      {
         //noise . . . no harm no foul
         noiseSquaredSum += value*value;
         ++noisePoints;
      }
   }

   //finish off the metrics (leaving values squared means db conversion is 10 * log10(squared_value) . . . or you can do 20 * log10(sqrt(squared_value)))
   if ((noiseSquaredSum + harmonicsSquaredSum) != 0)
   {
      m_sinad = fundamentalSquaredSum / (noiseSquaredSum + harmonicsSquaredSum);
   }
   m_sinad = 10 * log10(m_sinad) + dbMetricOffset;
   m_enob = (m_sinad - 1.76)/6.02;

   if (noiseSquaredSum != 0)
   {
      m_snr = fundamentalSquaredSum / noiseSquaredSum;
   }
   m_snr = 10 * log10(m_snr) + dbMetricOffset;


   if (harmonicsSquaredSum != 0)
   {
      m_thd = fundamentalSquaredSum / harmonicsSquaredSum;
   }
   m_thd = 10 * log10(m_thd) + dbMetricOffset;
   m_thd *= -1;

   //noise floor/top already offset properly due to scaling of fftDb
   m_noiseFloor = noiseFloor;
   m_noiseTop = noiseTop;
}



}
