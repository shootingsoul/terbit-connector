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

namespace terbit
{

 #define X(a, b) b,
 static char *TerbitDataTypeStrs[] = { TERBIT_DATA_TYPES };
 #undef X

LogToolsCategories g_logTools;

QString CompilerOptions()
{
   QString s;

#ifdef TERBIT_32BIT
   s.append("32-bit");
#endif
#ifdef TERBIT_64BIT
   s.append("64-bit");
#endif
#ifdef DEBUG
   s.append(" DEBUG");
#endif
#ifdef TEST
   s.append(" TEST");
#endif

   return s;
}

int CalculateDecimalPlaces(double value)
{
   //can only handle 10 decimal precision  . . .
   //may be an artificial limitation . . . seen it used elsewhere
   int decimals = 0;
   while (value - floor(value) > 1e-10)
   {
      ++decimals;
      value *= 10;
   }
   return decimals;
}

size_t TerbitDataTypeSize(TerbitDataType dataType)
{
   size_t elementSize;

   switch (dataType)
   {
   case TERBIT_INT64:
      elementSize = sizeof(int64_t);
      break;
   case TERBIT_UINT64:
      elementSize = sizeof(uint64_t);
      break;
   case TERBIT_INT32:
      elementSize = sizeof(int32_t);
      break;
   case TERBIT_UINT32:
      elementSize = sizeof(uint32_t);
      break;
   case TERBIT_INT16:
      elementSize = sizeof(int16_t);
      break;
   case TERBIT_UINT16:
      elementSize = sizeof(uint16_t);
      break;
   case TERBIT_INT8:
      elementSize = sizeof(int8_t);
      break;
   case TERBIT_UINT8:
      elementSize = sizeof(uint8_t);
      break;
   case TERBIT_FLOAT:
      elementSize = sizeof(float);
      break;
   case TERBIT_DOUBLE:
      elementSize = sizeof(double);
      break;      
    case TERBIT_SIZE_T:
      elementSize = sizeof(size_t);
      break;
   case TERBIT_BOOL:
      elementSize = sizeof(bool);
      break;
   case TERBIT_UTF8:
      elementSize = sizeof(char*); //assume dynamic allocation
      break;
   case TERBIT_DATETIME:
      elementSize = sizeof(uint64_t); //ms since epoch
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < dataType)
      {
         FatalError(g_logTools.general, QString("TerbitDataTypeSize Unknown data type - %1").arg(dataType));
      }
      else
      {
         LogWarning(g_logTools.general, QObject::tr("TerbitDataTypeSize invalid data type %1 (%2).").arg(dataType).arg(TerbitDataTypeStrs[dataType]));
      }
      elementSize = 1;
      break;
   }

   return elementSize;
}

void ToolsGlobalStartup()
{
   LogGlobalStartup();

   g_logTools.crypto = g_logManager.RegisterCategory("terbit.crypto");
   g_logTools.net = g_logManager.RegisterCategory("terbit.net");
   g_logTools.data = g_logManager.RegisterCategory("terbit.data");
   g_logTools.script = g_logManager.RegisterCategory("terbit.script");
   g_logTools.webServices = g_logManager.RegisterCategory("terbit.webservices");
   g_logTools.computerInfo = g_logManager.RegisterCategory("terbit.systeminfo");
   g_logTools.general = g_logManager.RegisterCategory("terbit.tools");
}

void ToolsGlobalShutdown()
{
   LogGlobalShutdown();
}

QString DoubleToStringComplete(double value)
{
   //generate full number value without fixed decimal places (max of 10)
   QString res = QString::number(value,'f',10);
   QChar c;
   int i = res.length()-1;
   while (i > 0)
   {
      c = res.at(i);
      if (c != '0' && c != '.')
      {
         break;
      }
      else if ( c == '.')
      {
         --i;
         break;
      }
      --i;
   }
   return res.left(i+1);
}

QString CompilerInfo()
{
   QString res;

#if defined(__clang__)
   /* Clang/LLVM. ---------------------------------------------- */   
   /* TRANSLATOR official name of compiler */
   res = QObject::tr("Clang/LLVM ") + QString(__VERSION__);
#elif defined(__ICC) || defined(__INTEL_COMPILER)
   /* Intel ICC/ICPC. ------------------------------------------ */

#elif defined(__GNUC__) || defined(__GNUG__)
   /* GNU GCC/G++. --------------------------------------------- */
   /* TRANSLATOR official name of compiler */
   res = QObject::tr("GNU GCC/G++ ") + QString(__VERSION__);

#elif defined(__HP_cc) || defined(__HP_aCC)
   /* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
   /* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
   /* Microsoft Visual Studio. --------------------------------- */
   /* TRANSLATOR official name of compiler with version separators.
      Example: "Microsoft Visual C++ 15.00.20017.01" */
   res = QObject::tr("Microsoft Visual C++ ") +
         QString::number(_MSC_FULL_VER).left(2)  + QObject::tr(".") +
         QString::number(_MSC_FULL_VER).mid(2,2) + QObject::tr(".") +
         QString::number(_MSC_FULL_VER).mid(4)   + QObject::tr(".") +
         QString::number(_MSC_BUILD);

#elif defined(__PGI)
   /* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
   /* Oracle Solaris Studio. ----------------------------------- */

#endif

   return res;
}

}
