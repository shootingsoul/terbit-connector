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
#include "Tools.h"

namespace terbit
{

class SignalTools
{
public:
   SignalTools();

   static void BoxcarWindow(double* filter, size_t filterLen);
   static void TriangleWindow(double* filter, size_t filterLen);
   static void GaussianWindow(double* filter, size_t filterLen, double alpha);
   static void TukeyWindow(double* filter, size_t filterLen, double r);
   static void HammingWindow(double* filter, size_t filterLen);
   static void HanningWindow(double* filter, size_t filterLen);

   static void Convolve(const double* signal, size_t signalLen, const double* kernel, size_t kernelLen, double* result, size_t resultLen);

};

}
