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

/**
HOW TO USE: FatalError and debugging macros

use FatalError for irrecoverable conditions, attempts to log before aborting program (alternative to using exceptions)

   FatalError(g_logTools.net, "Should never have gotten to this point.  Something is terribly wrong and unexpected.");

debugging macros output to stdout

   //to write out a deguggind message
   dmsg("dickey fuller was here")     //  displays "dickey fuller was here"

   //to write out variable values . . . handy as all get out
   int a = 5;
   int b = 4;
   dvar(a)                                           // displays "a = 5"
   dvar(a+b*4)                                   // displays "a+b*4 = 21"

*/

#pragma once

#include <QDebug>
#include <QString>
#include <iostream>
#include <stdexcept>
#include <limits.h>
#include <stdlib.h>
#include "Log.h"
#include "TerbitDefs.h"

namespace terbit
{


//determine if we are 32 or 64 bit, check unsigned long max
#if (ULONG_MAX > 4294967295) || _WIN64 || __amd64__
   #define TERBIT_64BIT
#else
   #define TERBIT_32BIT
#endif

#ifdef DEBUG
   //print out variable name and value
   #define dvar(v) qDebug() << QString(#v) << " = " << v;
   //print out msg
   #define dmsg(s) qDebug() << QString(s);
#else
	#define dvar(v)
	#define dmsg(s)
#endif

//try to log fatal error before exiting . . .
#if _WINDOWS
   #define __func__ __FUNCTION__
#endif // _WINDOWS

#define FatalError(category, msg) LogError(category, QString("Fatal Error: %1\nfunction: %2 at %3:%4").arg(QString(msg)).arg(__func__).arg(__FILE__).arg(__LINE__)); g_logManager.Close(); abort();

//Logging categories for tools
class LogToolsCategories
{
public:
   LogToolsCategories() {}
   uint32_t net;
   uint32_t crypto;
   uint32_t data;
   uint32_t script;
   uint32_t webServices;
   uint32_t computerInfo;
   uint32_t general;
};

extern LogToolsCategories g_logTools;


size_t TerbitDataTypeSize(TerbitDataType dataType);

//scaling functions to map unit ranges from logical (e.g. pixel) to data values
template<typename DataType, typename DataRangeType>
DataType ScaleLogicalToData(int pos, int min, int range, DataType dataMin, DataRangeType dataRange ) { return (DataType)(((pos - min)/ (long double)range) * dataRange) + dataMin; }

template<typename DataType, typename DataRangeType>
DataType ScaleLogicalReverseToData(int pos, int min, int range, DataType dataMin, DataRangeType dataRange ) { return (DataType)(((range - (pos - min))/ (long double)range) * dataRange) + dataMin; }

template<typename DataType, typename DataRangeType>
int ScaleDataToLogical(DataType value, int min, int range, DataType dataMin, DataRangeType dataRange ) { return floor((value - dataMin) / (long double)dataRange * range) + min; }

template<typename DataType, typename DataRangeType>
int ScaleDataToLogicalReverse(DataType value, int min, int range, DataType dataMin, DataRangeType dataRange ) { return range - floor((value - dataMin) /(long double)dataRange * range) + min; }

//reverse position in the range of values
inline int ReversePosition(int pos, int min, int range) { return range - (pos - min) + min; }

//deterrmine number of decimal places used in a double value
int CalculateDecimalPlaces(double value);
//expand full value of double to a string with all digits (no trailing zeros)
QString DoubleToStringComplete(double value);

QString CompilerOptions();
QString CompilerInfo();

void ToolsGlobalStartup();
void ToolsGlobalShutdown();

}



