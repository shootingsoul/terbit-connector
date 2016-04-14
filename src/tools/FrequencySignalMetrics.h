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

#include <vector>
#include "Tools.h"

namespace terbit
{

/*!
   -------------------------------
   Frequency analysis
   -------------------------------
   input fft signal is linear magnitude scaled properly (i.e. abs(fft(signal))*2/N, where N is original signal length and only includes points to nyquist (N/2+1))

   -dc is the first value

   -fundamental frequency is the highest value

   -harmonics occur at intervals of the fundamental frequency
    For example, if fundamental occurs at 13th element, then every 13th element is a harmonic
    We make sure the harmonics are above the noise floor (so we're now drowned in noise)

   -Noise is everything in signal except for the dc, fundamental and harmonics

   -values are converted to dB (decibels) by taking 20*log10(value/dBScale)
    dBScale may be dBc (carrier) or dBFS (full scale)

   ---------------------------
   SNR (Signal-to-noise)
   ---------------------------
   SNR = [Fundamental] / SQRT (SUM (SQR([Noise])))
   SNR_dB = 20 * log10 (SNR)

   ---------------------------
   THD (Total Harmonic Distortion)
   ---------------------------
   THD = SQRT (SUM (SQR ([Harmonics]))) / [Fundamental]
   THD_dB = 20 * log10 (THD)

   -------------------------------
   SFDR (Spurious Free Dynamic Range)
   -------------------------------
   (using fft signal converted to decibels)
   take the mean of the fft signal in decibels
   or the highest value that is not the fundamental (may be a harmonic) above the mean

   ---------------------------
   SINAD (Signal-to-noise and distortion ratio)
   ---------------------------
   SINAD = [Fundamental] / SQRT (SUM (SQR([Noise and Harmonics])))
   SINAD_dB = 20 * log10 (SINAD)

   -----------------------------
   ENOB (Effective number of bits)
   ---------------------------
   ENOB = (SINAD_dB - 1.76)/6.02
*/
class FrequencySignalMetrics
{
public:
   FrequencySignalMetrics();
   ~FrequencySignalMetrics();

   /*!
    * \brief Calculate
    * \param fft
    * \param elementCount
    * \param maxHarmonicCount number of harmonics to use (including fundamental, must be >= 1)
    * \param binsDC number points to exclude next to the DC from the calcs (must be >= 0)
    * \param binsFundamental number of points to exclude on either side of the fundamental point from the calcs (must be >= 0)
    * \param binsHarmonics number of points to exclude on either side of the harmonic points from the calcs (must be >= 0)
    * \param fullScale use dBc or dBFS
    * \param bits number of bits used for the original input signal, needed for dBFS calculation only
    * \param noiseRange decibels from the noise floor to the noise top to ensure harmonic values are valid
    */
   void Calculate(double* fft, double* fftDb, size_t elementCount, int maxHarmonicCount, int binsDC, int binsFundamental, int binsHarmonics, bool fullScale, int bits, double noiseRange);

   const double& GetSNR() { return m_snr; }
   const double& GetTHD() { return m_thd; }
   const double& GetSFDR() { return m_sfdr; }
   const double& GetSINAD() { return m_sinad; }
   const double& GetENOB() { return m_enob; }
   const double& GetNoiseFloor() { return m_noiseFloor; }
   const double& GetNoiseTop() { return m_noiseTop; }

   /*!
    * \brief return fundamental in dBFS so it's always available even if doing dBc for the other metrics
    */
   const double& GetFundamentalDecibels() { return m_fundamentalDecibels; }

   class HarmonicInfo
   {
      friend class FrequencySignalMetrics;
   public:
      HarmonicInfo();

      /*!
       * \brief index or 'bin' of the harmonic
       */
      size_t GetIndex() { return m_index; }

      /*!
       * \brief decibel amplitude value of the harmonic
       */
      double GetAmplitude() { return m_amplitude; }

   private:
      size_t m_index, m_startIndex, m_endIndex;
      double m_amplitude;
   };

   /*!
    * \brief Information for each harmonic including fundamental
    */
   const std::vector<HarmonicInfo*>& GetHarmonics() { return m_harmonics; }

private:
   void ClearHarmonics();
   void AddHarmonic(size_t index, size_t bins, size_t binsDC, size_t count);

   double m_snr, m_thd, m_sfdr, m_sinad, m_enob, m_noiseFloor, m_noiseTop, m_fundamentalDecibels;
   std::vector<HarmonicInfo*> m_harmonics;
};

}
