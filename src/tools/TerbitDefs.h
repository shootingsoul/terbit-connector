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

/*!
 *  Standard enums and typedefs shared by core and external plugin development
 */

#include <stdint.h>
#include <cstddef>

namespace terbit
{

typedef uint32_t BlockIOCategory_t;
typedef uint32_t DataClassAutoId_t;
typedef uint32_t DockId_t;

static const uint32_t BLOCK_IO_CATEGORY_GENERAL = 0;

// define a value that we will never return for a valid key.  This
// can be used to check for validity.
#define INVALID_ID_VAL 0

enum DataClassKind
{
   DATA_CLASS_KIND_SOURCE = 0,
   DATA_CLASS_KIND_DEVICE = 1,
   DATA_CLASS_KIND_DISPLAY = 2,
   DATA_CLASS_KIND_PROCESSOR = 3
};

#if 0
// Behold the power of the X macro!  See details at
// http://www.drdobbs.com/cpp/the-x-macro/228700289
enum TerbitDataType
{
   TERBIT_UINT8 = 0,
   TERBIT_UINT16,
   TERBIT_UINT32,
   TERBIT_UINT64,
   TERBIT_INT8,
   TERBIT_INT16,
   TERBIT_INT32,
   TERBIT_INT64,
   TERBIT_FLOAT,
   TERBIT_DOUBLE,
   TERBIT_SIZE_T,
   TERBIT_UTF8,
   TERBIT_BOOL,
   TERBIT_DATETIME
};
#else
#define TERBIT_DATA_TYPES      \
    X(TERBIT_INT8,   "int8")   \
    X(TERBIT_UINT8,  "uint8")  \
    X(TERBIT_INT16,  "int16")  \
    X(TERBIT_UINT16, "uint16") \
    X(TERBIT_INT32,  "int32")  \
    X(TERBIT_UINT32, "uint32") \
    X(TERBIT_INT64,  "int64")  \
    X(TERBIT_UINT64, "uint64") \
    X(TERBIT_FLOAT,  "float")  \
    X(TERBIT_DOUBLE, "double") \
    X(TERBIT_SIZE_T, "size_t") \
    X(TERBIT_BOOL,   "bool")   \
    X(TERBIT_UTF8,   "utf8")   \
    X(TERBIT_DATETIME, "datetime") \
    X(TERBIT_DATA_TYPE_GUARD, "invalidType")

#define X(a, b) a,
enum TerbitDataType { TERBIT_DATA_TYPES };
#undef X

// now, when you need string representations, add the following code
// to the .c file where it is needed:
// #define X(a, b) b,
// static char *TerbitDataTypeStrs[] = { TERBIT_DATA_TYPES };
// #undef X


static const char* TERBIT_TYPE_DATASET                               = "terbit.core.dataset";
static const char* TERBIT_TYPE_DATASET_VALUES_DISPLAY                = "terbit.displays.dataset-values";
static const char* TERBIT_TYPE_XYPLOT                                = "terbit.plots.xyplot";
static const char* TERBIT_TYPE_MICROPHONE                            = "terbit.microphone.microphone";
static const char* TERBIT_TYPE_FFT                                   = "terbit.signal-processing.fft";
static const char* TERBIT_TYPE_FREQUENCY_METRICS_DISPLAY             = "terbit.signal-processing.frequency-metrics-display";
static const char* TERBIT_TYPE_SCRIPT_PROCESSOR                      = "terbit.scripting.processor";
static const char* TERBIT_TYPE_SCRIPT_DISPLAY                        = "terbit.scripting.display";

//data properties
static const char* TERBIT_DATA_PROPERTY_SAMPLING_RATE = "SamplingRateHz"; // double, Hz
static const char* TERBIT_DATA_PROPERTY_SAMPLING_BITS = "BitsPerSample"; // uint32_t, n bits

#endif

}
