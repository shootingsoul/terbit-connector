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
#include "SignalTools.h"
#include "Tools.h"
namespace terbit
{

#ifndef M_PI
double M_PI = 4.0*atan(1.0);
#endif

SignalTools::SignalTools()
{

}

void SignalTools::BoxcarWindow(double *filter, size_t filterLen)
{
   for(size_t i = 0; i< filterLen; ++i, ++filter)
   {
      *filter = 1;
   }
}

void SignalTools::TriangleWindow(double *filter, size_t filterLen)
{

   if (filterLen % 2 == 0)
   {
      size_t L1 = filterLen - 1;
      size_t L12 = L1/2;
      //even
      for(size_t i = 0; i< filterLen; ++i, ++filter)
      {
         if (i <= L12)
         {
            *filter = (2*i) / (double)L1;
         }
         else
         {
            *filter = 2 - (2*i) / (double)L1;
         }
      }
   }
   else
   {
      //odd
      size_t L2 = filterLen/2;
      for(size_t i = 0; i< filterLen; ++i, ++filter)
      {
         if (i <= L2)
         {
            *filter = (2*i - 1) / (double)filterLen;
         }
         else
         {
            *filter = 2 - (2*i - 1) / (double)filterLen;
         }
      }
   }
}

void SignalTools::GaussianWindow(double *filter, size_t filterLen, double alpha)
{
   double N2 = (filterLen - 1)/2.0;

   for(size_t i = 0; i< filterLen; ++i, ++filter)
   {
      *filter = exp(-0.5*pow((alpha*(i/N2)),2.0));
   }
}

void SignalTools::TukeyWindow(double *filter, size_t filterLen, double r)
{
   double r2 = r/2.0;
   double PI2_R = (2*M_PI)/r;

   for(size_t i = 0; i< filterLen; ++i, ++filter)
   {
      if (i < r2)
      {
         *filter = 0.5*(1 + cos(PI2_R * (i - r2)));
      }
      else if (r < (1 - r2))
      {
         *filter = 1;
      }
      else
      {
         *filter = 0.5*(1 + cos(PI2_R * (i - 1 + r2)));
      }
   }
}

void SignalTools::HammingWindow(double *filter, size_t filterLen)
{
   double N = filterLen - 1;

   for(size_t i = 0; i< filterLen; ++i, ++filter)
   {
      *filter = 0.54-0.46*cos(2*M_PI*(i/N));
   }
}

void SignalTools::HanningWindow(double *filter, size_t filterLen)
{
   double N = filterLen - 1;

   for(size_t i = 0; i< filterLen; ++i, ++filter)
   {
      *filter = 0.5*(1-cos(2*M_PI*(i/N)));
   }
}

void SignalTools::Convolve(const double* signal, size_t signalLen, const double* kernel, size_t kernelLen, double* result, size_t resultLen)
{
   size_t n;

   if (signalLen == kernelLen)
   {
      if (resultLen > signalLen) //allow for result to be cropped
      {
         LogError(g_logTools.data,QObject::tr("Convolve invalid result length: %1.  Expected %2").arg(resultLen).arg(signalLen));
      }
      else
      {
         for(n = 0; n < resultLen; ++n, ++result, ++signal, ++kernel)
         {
            *result = *signal * *kernel;
         }
      }
   }
   else
   {
      if (resultLen > signalLen + kernelLen - 1) //allow result to be cropped
      {
         LogError(g_logTools.data,QObject::tr("Convolve invalid result length: %1.  Expected %2").arg(resultLen).arg(signalLen + kernelLen - 1));
      }
      else
      {
         for (n = 0; n < resultLen; ++n, ++result)
         {
            size_t kmin, kmax, k;

            *result = 0;

            kmin = (n >= kernelLen - 1) ? n - (kernelLen - 1) : 0;
            kmax = (n < signalLen - 1) ? n : signalLen - 1;

            for (k = kmin; k <= kmax; k++)
            {
               *result += signal[k] * kernel[n - k];
            }
         }
      }
   }
}

}
