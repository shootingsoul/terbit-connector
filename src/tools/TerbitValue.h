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

class TerbitValue
{
public:
   TerbitValue();
   ~TerbitValue();

   TerbitDataType GetDataType() const { return m_dataType; }

   void SetDataType(TerbitDataType dataType); //creates type with unitialized value
   void ConvertDataType(TerbitDataType dataType); //converts existing value to new type

   void SetValue(const TerbitValue& value);

   template<typename DataType>
   void SetValue(DataType newValue)
   {
      switch (m_dataType)
      {
      case TERBIT_UINT8:
         (*((uint8_t*)m_value)) = newValue;
         break;
      case TERBIT_INT8:
         (*((int8_t*)m_value)) = newValue;
         break;
      case TERBIT_UINT16:
         (*((uint16_t*)m_value)) = newValue;
         break;
      case TERBIT_INT16:
         (*((int16_t*)m_value)) = newValue;
         break;
      case TERBIT_UINT32:
         (*((uint32_t*)m_value)) = newValue;
         break;
      case TERBIT_INT32:
         (*((int32_t*)m_value)) = newValue;
         break;
      case TERBIT_UINT64:
         (*((uint64_t*)m_value)) = newValue;
         break;
      case TERBIT_INT64:
         (*((int64_t*)m_value)) = newValue;
         break;
      case TERBIT_FLOAT:
         (*((float*)m_value)) = newValue;
         break;
      case TERBIT_DOUBLE:
         (*((double*)m_value)) = newValue;
         break;
      }
   }

   void* GetValue() const { return m_value; }

   template<typename DataType>
   DataType GetConvertedValue() const
   {
      switch (m_dataType)
      {
      case TERBIT_UINT8:
         return (DataType)(*((uint8_t*)m_value));
         break;
      case TERBIT_INT8:
         return (DataType)(*((int8_t*)m_value));
         break;
      case TERBIT_UINT16:
         return (DataType)(*((uint16_t*)m_value));
         break;
      case TERBIT_INT16:
         return (DataType)(*((int16_t*)m_value));
         break;
      case TERBIT_UINT32:
         return (DataType)(*((uint32_t*)m_value));
         break;
      case TERBIT_INT32:
         return (DataType)(*((int32_t*)m_value));
         break;
      case TERBIT_UINT64:
         return (DataType)(*((uint64_t*)m_value));
         break;
      case TERBIT_INT64:
         return (DataType)(*((int64_t*)m_value));
         break;
      case TERBIT_FLOAT:
         return (DataType)(*((float*)m_value));
         break;
      case TERBIT_DOUBLE:
         return (DataType)(*((double*)m_value));
         break;
      }
   }

private:
   TerbitValue(const TerbitValue&);

   TerbitDataType m_dataType;
   void *m_value;
};


}
