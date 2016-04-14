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

#include "DataSet.h"
#include "LogDL.h"
#include "Workspace.h"
#include "Block.h"

namespace terbit
{

#define X(a, b) b,
static char *TerbitDataTypeStrs[] = { TERBIT_DATA_TYPES };
#undef X

DataSet::DataSet(): DataSource(), m_buffer(NULL), m_strideBytes(0),
   m_managedBufferSize(0), m_managedBuffer(false), m_inputSource(this), m_indexDataSet(NULL)
{
   //NOTE: default input source as this dataset
}

DataSet::~DataSet()
{
   if (m_managedBuffer)
   {
      free(m_buffer);
   }
}

bool DataSet::ImpendingDeletion()
{
   //auto remove index if it's a remote
   if (m_indexDataSet && m_indexDataSet->GetOwner() == this)
   {
      GetWorkspace()->DeleteInstance(m_indexDataSet->GetAutoId());
   }

   return DataClass::ImpendingDeletion();
}

bool DataSet::IsRemote()
{
   return (this != m_inputSource);
}

bool DataSet::IsDataSet() const
{
   return true;
}

bool DataSet::Init()
{
   //datasets are readable/writable by default
   SetReadable(true);
   SetWritable(true);
   return DataSource::Init();
}

bool DataSet::InteractiveInit()
{
   //auto-load data for remotes on interactive init
   if (CanRefreshData())
   {
      RefreshData();
   }
   return true;
}

bool DataSet::InteractiveFocus()
{
   ShowDisplayView();
   return true;
}

void DataSet::SetIndexDataSet(DataSet *indexDataSet)
{
   if (m_indexDataSet)
   {
      disconnect(m_indexDataSet, SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeIndexRemoved(DataClass*)));
   }
   m_indexDataSet = indexDataSet;
   if (m_indexDataSet)
   {
      connect(m_indexDataSet, SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeIndexRemoved(DataClass*)));
   }
   emit IndexAssigned(this);
}

void DataSet::SetInputSource(DataSource* newSource)
{
   if (m_inputSource != this)
   {
      //disconnect(m_inputSource,SIGNAL(NewData(DataClass*)), this,SLOT(OnInputSourceNewData(DataClass*)));
      //disconnect(m_inputSource,SIGNAL(StructureChanged(DataSource*)),this, SLOT(OnInputSourceStructureChanged(DataSource*)));
      disconnect(m_inputSource,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeInputSourceRemoved(DataClass*)));
   }
   m_inputSource = newSource;
   if (m_inputSource != this)
   {
      //TODO *** sync options for SourceDataChanged . . . may or may not want to sync with the event
      //for now, no syncing of structure or new data changes
      //connect(m_inputSource,SIGNAL(NewData(DataClass*)), this,SLOT(OnInputSourceNewData(DataClass*)));
      //connect(m_inputSource,SIGNAL(StructureChanged(DataSource*)),this, SLOT(OnInputSourceStructureChanged(DataSource*)));
      connect(m_inputSource,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeInputSourceRemoved(DataClass*)));
   }

   emit InputSourceAssigned(this);
}

void DataSet::ShowValuesView()
{
   Block* d = static_cast<Block*>(GetWorkspace()->CreateInstance(TERBIT_TYPE_DATASET_VALUES_DISPLAY, NULL));
   d->InteractiveInit();
   d->ApplyInputDataClass(this);
}

void DataSet::ShowDisplayView()
{
   if (GetDisplayViewTypeName().length() > 0)
   {
      DataClass* dc = GetWorkspace()->CreateInstance(GetDisplayViewTypeName(), NULL);
      if (dc && dc->GetType()->GetKind() == DATA_CLASS_KIND_DISPLAY)
      {
         Block* d = static_cast<Block*>(dc);
         d->SetName(GetName());
         d->ApplyInputDataClass(this);
      }
      else
      {
         LogError2(GetType()->GetLogCategory(),GetName(),tr("Failed to create display of type '%1'").arg(GetDisplayViewTypeName()));
      }
   }
}

void DataSet::GetDirectDependencies(std::list<DataClass *> &dependsOn)
{
   DataClass* d = NULL;
   if (IsRemote())
   {
      //remotes depend on the source owner
      d = m_inputSource->GetOwner();
   }
   else
   {
     //non-remotes depend on their owner, unless the owner is a data set (e.g. index)
      d = GetOwner();
      if (d && d->IsDataSet())
      {
         d = NULL;
      }
   }

   if (d != NULL)
   {
      dependsOn.push_back(d);
   }
}

void DataSet::OnInputSourceNewData(DataClass* source)
{
   source;
   //when syncing data with a remote source . . .auto-refresh
   RefreshData();
}

bool DataSet::CanRefreshData()
{
   return (m_inputSource != this && m_inputSource->GetReadable());
}

void DataSet::RefreshData()
{
   if (CanRefreshData()) //paranoid check
   {
      //for now assume we are mirror of source . . . so update data set in-case the source structure data type changed . . .
      //future sync option perhaps . . .
      if (m_inputSource->GetDataType() != GetDataType())
      {
         CreateBuffer(GetDataType(),GetFirstIndex(),GetCount());
      }

      m_inputSource->ReadRequest(GetFirstIndex(), GetCount(), GetAutoId());

      //automatically refresh index too
      if (m_indexDataSet != NULL && m_indexDataSet->CanRefreshData())
      {
         m_indexDataSet->RefreshData();
      }
   }
   else
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("Attempt to refresh data on a data set that is not refreshable."));
   }
}

void DataSet::OnBeforeInputSourceRemoved(DataClass* source)
{
   source;
   //our input source is being removed . . . so revert to self-sourced
   if (m_inputSource != this)
   {
      SetInputSource(this);
   }
}

void DataSet::OnBeforeIndexRemoved(DataClass *idx)
{
   idx;
   m_indexDataSet = NULL;
}

void DataSet::OnInputSourceStructureChanged(DataSource *source)
{
   source;
   //we're a remote data set . . . sync our structure to the remote input source
   //TODO?? use default buffer elements or existing buffer size????
   CreateBuffer(m_inputSource->GetDataType(),m_inputSource->GetFirstIndex(),m_inputSource->GetDefaultBufferElements());
}

void DataSet::SetBuffer(TerbitDataType type, uint64_t firstIndex, size_t elementCount, void* bufferAddress, size_t strideBytes)
{
   //setup as unmanaged buffer
   if (m_managedBuffer)
   {
      free(m_buffer);
      m_managedBufferSize = 0;
      m_managedBuffer = false;
   }

   m_buffer = bufferAddress;
   m_strideBytes = strideBytes;
   m_defaultBufferElements = elementCount;
   UpdateStructure(type,firstIndex,elementCount);
}

void DataSet::CreateBuffer(TerbitDataType type, uint64_t firstIndex, size_t elementCount)
{
   //create managed buffer
   size_t elementSize = TerbitDataTypeSize(type);

   if (m_managedBuffer)
   {
      if (elementSize*elementCount > m_managedBufferSize)
      {
         free(m_buffer);
         m_managedBufferSize = elementCount*elementSize;
         m_buffer = malloc(elementCount*elementSize);
         if (!m_buffer)
         {
            FatalError(g_log.general, QObject::tr("DataSet::CreateBuffer Memory Allocation Failure"));
         }
      }
   }
   else
   {
      m_managedBuffer = true;
      m_managedBufferSize = elementCount*elementSize;
      m_buffer = malloc(elementCount*elementSize);
      if (!m_buffer)
      {
         FatalError(g_log.general, QObject::tr("DataSet::CreateBuffer Memory Allocation Failure"));
      }
   }

   m_strideBytes = elementSize;
   m_defaultBufferElements = elementCount;
   UpdateStructure(type,firstIndex, elementCount);
}

void DataSet::CreateBufferAsIndex(uint64_t firstIndex, size_t elementCount)
{
#ifdef TERBIT_64BIT
   CreateBuffer(TERBIT_UINT64, firstIndex, elementCount);
   uint64_t* buf = (uint64_t*)GetBufferAddress();
#else
   CreateBuffer(TERBIT_UINT32, firstIndex, elementCount);
   uint32_t* buf = (uint32_t*)GetBufferAddress();
#endif
   for(size_t i = 0; i < elementCount; ++i)
   {
      *buf = i;
      ++buf;
   }
}

void DataSet::ReadRequest(uint64_t startIndex, size_t elementCount, DataClassAutoId_t dataSetId)
{
   if (GetHasData() == false)
   {
      LogWarning2(GetType()->GetLogCategory(),GetName(),tr("Read request on source with no data.  The data set/element count may be setup, but there is no valid data in the source data set to be read."));
      return;
   }

   //validate start index
   uint64_t sourceEndElement = GetFirstIndex() + GetCount();
   if (!(startIndex >= GetFirstIndex() && startIndex < sourceEndElement))
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("ReadRequest error.  The start index is out of range. Start index %1 not in range of first index %2 count %3").arg(startIndex).arg(GetFirstIndex()).arg(GetCount()));
      return;
   }

   //validate elementCount . . . we'll be nice and reduce if it's too big instead of error
   //or should we error??
   if (startIndex + elementCount > sourceEndElement)
   {
      LogWarning2(GetType()->GetLogCategory(),GetName(),tr("Read request element count is too long.  Returning less data than requested."));
      elementCount = sourceEndElement - startIndex;
   }

   //get destination
   DataSet* destDS = static_cast<DataSet*>(GetWorkspace()->FindInstance(dataSetId));
   if (destDS == NULL)
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("Read Request destination data set not found.  Destination data set id: %1").arg(dataSetId));
      return;
   }

   DataSet* dest =  destDS;
   if (!dest->GetWritable())
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("ReadRequest error.  The destination data set is not writable.  Destination data set: %1").arg(destDS->GetName()));
      return;
   }

   if (elementCount > dest->GetCount())
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("ReadRequest error.  The destination data set buffer is too small for the requested read count.  Destination data set=%1 buffer elements=%2 Requested Read=%3").arg(destDS->GetName()).arg(dest->GetCount()).arg(elementCount));
      return;
   }

   if (dest->GetDataType() != GetDataType())
   {
      LogError2(GetType()->GetLogCategory(),GetName(),tr("ReadRequest error.  The data set and destination data types do no match.  Data type conversion is not currently supported.  Destination data set: %1").arg(destDS->GetName()));
      return;
   }

   //copy the data
   size_t elementSize = TerbitDataTypeSize(GetDataType());   
   memcpy((char*)dest->GetBufferAddress(), (char*)GetBufferAddress() + (startIndex-GetFirstIndex())*elementSize, elementCount * elementSize);

   //also read properties of data
   //properties should by in sync with data so we read them together
   dest->GetProperties() = m_properties;

   dest->SetHasData(true);
   dest->UpdateStructure(dest->GetDataType(),startIndex, elementCount);
   emit dest->NewData(dest);
}


template<typename DataType>
DataType ValueAtIndexTemplate(size_t index, char* data, size_t strideBytes)
{
   return (*((DataType*)((data)+index*strideBytes)));
}

double DataSet::GetValueAtIndex(size_t index) const
{
   double dataPoint;

   switch (m_dataType)
   {
   case TERBIT_INT64:
      dataPoint = ValueAtIndexTemplate<int64_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_UINT64:
      dataPoint = ValueAtIndexTemplate<uint64_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_INT32:
      dataPoint = ValueAtIndexTemplate<int32_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_UINT32:
      dataPoint = ValueAtIndexTemplate<uint32_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_INT16:
      dataPoint = ValueAtIndexTemplate<int16_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_UINT16:
      dataPoint = ValueAtIndexTemplate<uint16_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_INT8:
      dataPoint = ValueAtIndexTemplate<int8_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_UINT8:
      dataPoint = ValueAtIndexTemplate<uint8_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_FLOAT:
      dataPoint = ValueAtIndexTemplate<float>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_DOUBLE:
      dataPoint = ValueAtIndexTemplate<double>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_SIZE_T:
      dataPoint = ValueAtIndexTemplate<size_t>(index, (char*) m_buffer, m_strideBytes);
      break;
   case TERBIT_BOOL:
      dataPoint = ValueAtIndexTemplate<bool>(index, (char*) m_buffer, m_strideBytes);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < m_dataType)
      {
         FatalError(g_log.general, QString("GetValueAtIndex Unknown data type - %1").arg(m_dataType));
      }
      else
      {
         LogWarning(g_log.general, tr("Cannot convert data type %1 (%2) to double.").arg(m_dataType).arg(TerbitDataTypeStrs[m_dataType]));
      }
      dataPoint = 0;
      break;
   }

   return dataPoint;
}

template<typename DataType>
void SetValueAtIndexTemplate(size_t index, char* data, size_t strideBytes, double value)
{
   (*((DataType*)((data)+index*strideBytes))) = (DataType)value;
}

void DataSet::SetValueAtIndex(size_t index, double value)
{
   switch (m_dataType)
   {
   case TERBIT_INT64:
      SetValueAtIndexTemplate<int64_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_UINT64:
      SetValueAtIndexTemplate<uint64_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_INT32:
      SetValueAtIndexTemplate<int32_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_UINT32:
      SetValueAtIndexTemplate<uint32_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_INT16:
      SetValueAtIndexTemplate<int16_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_UINT16:
      SetValueAtIndexTemplate<uint16_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_INT8:
      SetValueAtIndexTemplate<int8_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_UINT8:
      SetValueAtIndexTemplate<uint8_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_FLOAT:
      SetValueAtIndexTemplate<float>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_DOUBLE:
      SetValueAtIndexTemplate<double>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_SIZE_T:
      SetValueAtIndexTemplate<size_t>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   case TERBIT_BOOL:
      SetValueAtIndexTemplate<bool>(index, (char*) m_buffer, m_strideBytes, value);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < m_dataType)
      {
         FatalError(g_log.general, QString("SetValueAtIndex Unknown data type - %1").arg(m_dataType));
      }
      else
      {
         LogWarning(g_log.general, tr("Cannot convert data type %1 (%2) to double.").arg(m_dataType).arg(TerbitDataTypeStrs[m_dataType]));
      }
      break;
   }
}

double DataSet::GetValueAtLogicalIndex(uint64_t index) const
{
   return GetValueAtIndex((size_t)(index - m_firstIndex));
}

void DataSet::SetValueAtLogicalIndex(uint64_t index, double value)
{
   SetValueAtIndex((size_t)(index - m_firstIndex), value);
}

template<typename DataType>
void LowerBoundIndexTemplate(double key, size_t& index, char* data, size_t strideBytes, size_t count)
{
   //find the bound
   DataType value;
   size_t left, right, mid;
   left = 0;
   right = count-1;
   while (left < right)
   {
      mid = left + (right - left) / 2;
      value = ValueAtIndexTemplate<DataType>(mid, data, strideBytes);
      if (value < key)
      {
         left = mid + 1;
      }
      else
      {
         right = mid;
      }
   }

   //now make sure value we found is on proper side
   //left is set properly
   //because values are not exact, could be before or after bound
   value = ValueAtIndexTemplate<DataType>(left, data, strideBytes); //ensure value is for left
   if (value <= key)
   {
      index = left;
   }
   else
   {
      index = left - 1;
   }
}

template<typename DataType>
void UpperBoundIndexTemplate(double key, size_t& index, char* data, size_t strideBytes, size_t count)
{
   //find the bound
   DataType value;
   size_t left, right, mid;
   left = 0;
   right = count-1;
   while (left < right)
   {
      mid = left + (right - left) / 2;
      value = ValueAtIndexTemplate<DataType>(mid, data, strideBytes);
      if (value < key)
      {
         left = mid + 1;
      }
      else
      {
         right = mid;
      }
   }

   //now make sure value we found is on proper side
   //left is set properly
   //because values are not exact, could be before or after bound
   value = ValueAtIndexTemplate<DataType>(left, data, strideBytes); //ensure value is for left
   if (value >= key)
   {
      index = left;
   }
   else
   {
      index = left+1;
   }
}

template<typename DataType>
bool BoundingIndiciesTemplate(double startValue, double endValue, size_t& start, size_t& end, char* data, size_t strideBytes, size_t count)
{
   //assume data is ordered
   //find indicies that tightly cover given value range
   //ensure that there's overlap with given value range
   bool foundStart, foundEnd;
   foundStart = foundEnd = false;

   if (count > 0)
   {
      DataType firstValue, lastValue;
      firstValue = ValueAtIndexTemplate<DataType>(0, data, strideBytes);
      lastValue = ValueAtIndexTemplate<DataType>(count-1, data, strideBytes);

      //check that there's overlap, may be outside range entirely
      if (firstValue >= startValue && firstValue < endValue)
      {
         start = 0;
         foundStart = true;
      }
      else if (firstValue < startValue)
      {
         //find start, outside of start value
         LowerBoundIndexTemplate<DataType>(startValue, start, data, strideBytes, count);
         foundStart = true;
      }

      //check that there's overlap, may be outside range entirely
      if (lastValue > startValue && lastValue <= endValue)
      {
         end = count-1;
         foundEnd = true;
      }
      else if (lastValue > endValue)
      {
         //find end, outside of end value
         UpperBoundIndexTemplate<DataType>(endValue, end, data, strideBytes, count);
         foundEnd = true;
      }
   }

   return foundStart && foundEnd;
}

bool DataSet::BoundingIndicies(double startValue, double endValue, size_t& start, size_t& end) const
{
   bool res = false;

   switch (m_dataType)
   {
   case TERBIT_INT64:
      res = BoundingIndiciesTemplate<int64_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT64:
      res = BoundingIndiciesTemplate<uint64_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT32:
      res = BoundingIndiciesTemplate<int32_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT32:
      res = BoundingIndiciesTemplate<uint32_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT16:
      res = BoundingIndiciesTemplate<int16_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT16:
      res = BoundingIndiciesTemplate<uint16_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT8:
      res = BoundingIndiciesTemplate<int8_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT8:
      res = BoundingIndiciesTemplate<uint8_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_FLOAT:
      res = BoundingIndiciesTemplate<float>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_DOUBLE:
      res = BoundingIndiciesTemplate<double>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_SIZE_T:
      res = BoundingIndiciesTemplate<size_t>(startValue, endValue, start, end, (char*)m_buffer, m_strideBytes, m_count);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < m_dataType)
      {
         FatalError(g_log.general, QString("ClosestIndex Unknown data type - %1").arg(m_dataType));
      }
      else
      {
         LogWarning(g_log.general, tr("ClosestIndex invalid data type %1 (%2).").arg(m_dataType).arg(TerbitDataTypeStrs[m_dataType]));
      }
      break;
   }

   return res;
}


template<typename DataType, typename KeyDataType>
bool ClosestIndexTemplate(KeyDataType key, size_t& index, char* data, size_t strideBytes, size_t count)
{
   //assume data is ordered
   //key must be within range
   //binary search for left/right index
   bool res = false;

   if (count > 0)
   {
      DataType value;
      size_t left, right, mid;
      left = 0;
      right = count-1;
      while (left < right)
      {
         mid = left + (right - left) / 2;
         value = ValueAtIndexTemplate<DataType>(mid, data, strideBytes);
         if (value < key)
         {
            left = mid + 1;
         }
         else
         {
            right = mid;
         }
      }

      //now make sure key is within range and determine which is closest
      //left is set properly
      //because values are not exact, could be before or after value
      value = ValueAtIndexTemplate<DataType>(left, data, strideBytes); //ensure value is for left
      size_t alt = left;
      if (value < key && left < count-1)
      {
         ++alt;
      }
      else if (left > 0)
      {
         --alt;
      }

      if (alt != left)
      {
         DataType valueAlt = ValueAtIndexTemplate<DataType>(alt, data, strideBytes);

         if (((value < valueAlt) && ((key - value) > (valueAlt - key))) ||
             ((value > valueAlt) && ((value - key) > (key - valueAlt))))
         {
            index = alt;
            value = valueAlt;
         }
         else
         {
            index = left;
         }
      }
      else
      {
         index = left;
      }

      //value and index set to closest
      //bounds checking to ensure value in range
      if (index == 0 && key < value)
      {
         res = false;
      }
      else if (index == count-1 && key > value)
      {
         res = false;
      }
      else
      {
         res = true;
      }
   }
   return res;
}

template<typename DataType>
bool ClosestIndexKeyTemplate(const TerbitValue& key, size_t& index, char* data, size_t strideBytes, size_t count)
{
   bool res = false;
   TerbitDataType t = key.GetDataType();

   switch (t)
   {
   case TERBIT_INT64:
      res = ClosestIndexTemplate<DataType, int64_t>(*((int64_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_UINT64:
      res = ClosestIndexTemplate<DataType, uint64_t>(*((uint64_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_INT32:
      res = ClosestIndexTemplate<DataType, int32_t>(*((int32_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_UINT32:
      res = ClosestIndexTemplate<DataType, uint32_t>(*((uint32_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_INT16:
      res = ClosestIndexTemplate<DataType, int16_t>(*((int16_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_UINT16:
      res = ClosestIndexTemplate<DataType, uint16_t>(*((uint16_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_INT8:
      res = ClosestIndexTemplate<DataType, int8_t>(*((int8_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_UINT8:
      res = ClosestIndexTemplate<DataType, uint8_t>(*((uint8_t*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_FLOAT:
      res = ClosestIndexTemplate<DataType, float>(*((float*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_DOUBLE:
      res = ClosestIndexTemplate<DataType, double>(*((double*)key.GetValue()), index, data, strideBytes, count);
      break;
   case TERBIT_SIZE_T:
      res = ClosestIndexTemplate<DataType, size_t>(*((double*)key.GetValue()), index, data, strideBytes, count);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < t)
      {
         FatalError(g_log.general, QString("ClosestIndex Unknown key data type - %1").arg(t));
      }
      else
      {
         LogWarning(g_log.general, QObject::tr("ClosestIndex invalid key data type %1 (%2).").arg(t).arg(TerbitDataTypeStrs[t]));
      }
      break;
   }

   return res;
}

bool DataSet::ClosestIndex(const TerbitValue& key, size_t& index) const
{
   bool res = false;

   switch (m_dataType)
   {
   case TERBIT_INT64:
      res = ClosestIndexKeyTemplate<int64_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT64:
      res = ClosestIndexKeyTemplate<uint64_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT32:
      res = ClosestIndexKeyTemplate<int32_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT32:
      res = ClosestIndexKeyTemplate<uint32_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT16:
      res = ClosestIndexKeyTemplate<int16_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT16:
      res = ClosestIndexKeyTemplate<uint16_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT8:
      res = ClosestIndexKeyTemplate<int8_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT8:
      res = ClosestIndexKeyTemplate<uint8_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_FLOAT:
      res = ClosestIndexKeyTemplate<float>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_DOUBLE:
      res = ClosestIndexKeyTemplate<double>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_SIZE_T:
      res = ClosestIndexKeyTemplate<size_t>(key, index, (char*)m_buffer, m_strideBytes, m_count);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < m_dataType)
      {
         FatalError(g_log.general, QString("ClosestIndex Unknown data type - %1").arg(m_dataType));
      }
      else
      {
         LogWarning(g_log.general, tr("ClosestIndex invalid data type %1 (%2).").arg(m_dataType).arg(TerbitDataTypeStrs[m_dataType]));
      }
      break;
   }

   return res;
}



template<typename DataType>
void CalculateMinMaxTemplate(TerbitValue& min, TerbitValue& max, char* buffer, size_t strideBytes, size_t count)
{
   if (count > 0)
   {
      char *d, *end;
      DataType mn, mx, value;
      d = buffer;
      end = d+count*strideBytes;

      mn = mx = *((DataType*)d);
      d += strideBytes;

      while (d!=end)
      {
         value = *((DataType*)d);
         if (value < mn)
         {
            mn = value;
         }
         else if (value > mx)
         {
            mx = value;
         }
         d += strideBytes;
      }
      min.SetValue<DataType>(mn);
      max.SetValue<DataType>(mx);
   }
}

void DataSet::CalculateMinMax(TerbitValue& min, TerbitValue& max) const
{
   switch (m_dataType)
   {
   case TERBIT_INT64:
      CalculateMinMaxTemplate<int64_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT64:
      CalculateMinMaxTemplate<uint64_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT32:
      CalculateMinMaxTemplate<int32_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT32:
      CalculateMinMaxTemplate<uint32_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT16:
      CalculateMinMaxTemplate<int16_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT16:
      CalculateMinMaxTemplate<uint16_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_INT8:
      CalculateMinMaxTemplate<int8_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_UINT8:
      CalculateMinMaxTemplate<uint8_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_FLOAT:
      CalculateMinMaxTemplate<float>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_DOUBLE:
      CalculateMinMaxTemplate<double>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_SIZE_T:
      CalculateMinMaxTemplate<size_t>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   case TERBIT_BOOL:
      CalculateMinMaxTemplate<bool>(min, max, (char*) m_buffer, m_strideBytes, m_count);
      break;
   default:
      if(TERBIT_DATA_TYPE_GUARD < m_dataType)
      {
         FatalError(g_log.general, QString("CalculateMinMax Unknown data type - %1").arg(m_dataType));
      }
      else
      {
         LogWarning(g_log.general, tr("CalculateMinMax invalid data type %1 (%2).").arg(m_dataType).arg(TerbitDataTypeStrs[m_dataType]));
      }
      break;
   }
}

bool ClosestDataPoint(const DataSet* X, const DataSet* Y, double dataX, double& pointX, double& pointY)
{
   bool res = false;
   size_t index;

   TerbitValue v;
   v.SetDataType(TERBIT_DOUBLE);
   v.SetValue(dataX);
   if (X->ClosestIndex(v,index))
   {
      pointX = X->GetValueAtIndex(index);
      pointY = Y->GetValueAtIndex(index);
      res = true;
   }

   return res;
}

DataSet* CreateRemoteDataSet(DataSource* source, DataClass *owner, bool publicScope)
{
   if (!source->GetReadable())
   {
      LogError2(source->GetType()->GetLogCategory(), source->GetName(),QObject::tr("Can't create a data set from a source that's not readable."));
      return NULL;
   }

   DataSet* res;

   if (source->IsDataSet())
   {
      auto ds = static_cast<DataSet*>(source);
      //this data ser is the source for the new data set
      res = ds->GetWorkspace()->CreateDataSet(owner, publicScope);
      res->CreateBuffer(ds->GetDataType(),ds->GetFirstIndex(),ds->GetDefaultBufferElements());
      res->SetDisplayViewTypeName(ds->GetDisplayViewTypeName());
      res->SetInputSource(ds);
      res->SetName(QObject::tr("%1 Remote").arg(ds->GetName()));

      //copy index
      DataSet* sourceIdx = ds->GetIndexDataSet();
      if (sourceIdx)
      {
         //by default same options for index . . .
         //parent of index is usually the data set it's indexing
         DataSet* idx;
         idx = CreateRemoteDataSet(sourceIdx, res, publicScope);
         res->SetIndexDataSet(idx);
      }
   }
   else
   {
      //data source
      res = source->GetWorkspace()->CreateDataSet(owner, publicScope);
      res->SetName(QObject::tr("%1 Data").arg(source->GetName()));
      res->SetInputSource(source);
      res->CreateBuffer(source->GetDataType(), source->GetFirstIndex(), source->GetDefaultBufferElements());
      res->SetDisplayViewTypeName(source->GetDisplayViewTypeName());
   }

   return res;
}

QObject *DataSet::CreateScriptWrapper(QJSEngine* se)
{
   return new DataSetSW(se, this);
}

ScriptDocumentation* BuildScriptDocumentationDataSet()
{
   ScriptDocumentation* d = BuildScriptDocumentationDataSource();

   d->SetSummary(QObject::tr("A data set represents a strided array of data with additional meta-data."));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetDisplayTypName"), "GetDisplayTypeName();",QObject::tr("Default display full type name string.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("CreateBuffer"), "CreateBuffer(dataType, firstIndex, elementCount);",QObject::tr("Create a managed buffer for the dataType (enum), firstIndex and elementCount.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetIndexDataSet"), "GetIndexDataSet();",QObject::tr("Returns a reference to the index data set.  If there is no index, then it returns undefined.  This allows indicies to represent values.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetIndexDataSet"), "SetIndexDataSet(ds);",QObject::tr("Set the data set to use for the index values.  The ds may be a reference or unique id string.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetValue"), "GetValue(index);",QObject::tr("Returns value for 0-based index in the data set.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetValue"), "SetValue(index, value);",QObject::tr("Sets the value at the 0-based index in the data set.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetValueLogicalIndex"), "GetValueLogicalIndex(index);",QObject::tr("Returns value for logical index in the data set based on the firstIndex offset for the data set.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetValueLogicalIndex"), "SetValueLogicalIndex(index, value);",QObject::tr("Sets the value at the logical index in the data set based on the firstIndex offset.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetInputDataSource"), "GetInputDataSource();",QObject::tr("Returns a reference to the input data source for this data set.  This is a self-reference when the data set does not have a remote data source.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetInputDataSource"), "SetInputDataSource(source);",QObject::tr("Sets the input data source for this data set.  This may be a reference to the data source or the unique id string.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("IsRemote"), "IsRemote();",QObject::tr("Returns a boolean if this data set has a remote data source (i.e. a data source that is not itself)")));

   d->AddScriptlet(new Scriptlet(QObject::tr("CanRefreshData"), "CanRefreshData();",QObject::tr("Returns a boolean if this data set can request to refresh its data from its remote data source.  This validates permissions.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("RefreshData"), "RefreshData();",QObject::tr("Requests data from the data source to update the data set.  The NewData event will fire when the data arrives.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("ShowValuesWindow"), "ShowValuesWindow();",QObject::tr("Show the data values display window.  This window displays all elements in the data set with options for hex display and more.")));

   return d;
}

DataSetSW::DataSetSW(QJSEngine* se, DataSet *ds) : DataSourceSW(se, ds)
{
}

void DataSetSW::CreateBuffer(int dataType, double firstIndex, double elementCount)
{
   DataSet* ds = static_cast<DataSet*>(m_dataClass);
   ds->CreateBuffer((TerbitDataType)dataType,firstIndex,elementCount);
}

QJSValue DataSetSW::GetIndexDataSet()
{
   //find public kid data set with that source
   QJSValue res;

   DataSet* ds = static_cast<DataSet*>(m_dataClass);
   if (ds->GetIndexDataSet())
   {
      res = m_scriptEngine->newQObject(ds->GetIndexDataSet()->CreateScriptWrapper(m_scriptEngine));
   }

   return res;
}

void DataSetSW::SetIndexDataSet(const QJSValue &ds)
{
   DataClass* dc = m_dataClass->GetWorkspace()->FindInstance(ds);
   if (dc && dc->IsDataSet())
   {
      DataSet* ds = static_cast<DataSet*>(m_dataClass);
      ds->SetIndexDataSet(static_cast<DataSet*>(dc));
   }
}

QJSValue DataSetSW::GetValue(double index)
{
   QJSValue res;
   if (BoundsCheck(index))
   {
      res = static_cast<DataSet*>(m_dataClass)->GetValueAtIndex((size_t)index);
   }
   return res;
}

void DataSetSW::SetValue(double index, double value)
{
   if (BoundsCheck(index))
   {
      static_cast<DataSet*>(m_dataClass)->SetValueAtIndex(index, value);
   }
}

QJSValue DataSetSW::GetValueLogicalIndex(double index)
{
   QJSValue res;

   if (BoundsCheckLogical(index))
   {
      res = static_cast<DataSet*>(m_dataClass)->GetValueAtLogicalIndex(index);
   }

   return res;
}

void DataSetSW::SetValueLogicalIndex(double index, double value)
{
   if (BoundsCheckLogical(index))
   {
      static_cast<DataSet*>(m_dataClass)->SetValueAtLogicalIndex(index, value);
   }
}

QJSValue DataSetSW::CalculateMinMax()
{
   double min,max;
   TerbitValue tmin, tmax;
   tmin.SetDataType(TERBIT_DOUBLE);
   tmax.SetDataType(TERBIT_DOUBLE);
   static_cast<DataSet*>(m_dataClass)->CalculateMinMax(tmin, tmax);
   min = *((double*)tmin.GetValue());
   max = *((double*)tmax.GetValue());

   QJSValue res = m_scriptEngine->newArray(2);
   //TODO convert to array
   //res.se = QJSValue(min);
   //res[1] = QJSValue(max);
   return res;
}

QJSValue DataSetSW::GetInputDataSource()
{
   auto ds = static_cast<DataSet*>(m_dataClass);
   return m_scriptEngine->newQObject(ds->GetInputSource().CreateScriptWrapper(m_scriptEngine));
}

void DataSetSW::SetInputDataSource(const QJSValue &source)
{
   DataClass* dc = m_dataClass->GetWorkspace()->FindInstance(source);
   if (dc && dc->GetType()->GetKind() == DATA_CLASS_KIND_SOURCE)
   {
        static_cast<DataSet*>(m_dataClass)->SetInputSource(static_cast<DataSource*>(dc));
   }
   else
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(), m_dataClass->GetName(), tr("Invalid 'source' parameter passed to SetInputDataSource"));
   }
}

bool DataSetSW::IsRemote()
{
   return static_cast<DataSet*>(m_dataClass)->IsRemote();
}

bool DataSetSW::CanRefreshData()
{
   return static_cast<DataSet*>(m_dataClass)->CanRefreshData();
}

void DataSetSW::RefreshData()
{
   static_cast<DataSet*>(m_dataClass)->RefreshData();
}

void DataSetSW::ShowValuesWindow()
{
   static_cast<DataSet*>(m_dataClass)->ShowValuesView();
}

bool DataSetSW::BoundsCheck(double index)
{
   auto ds = static_cast<DataSet*>(m_dataClass);
   if (index < 0 || index >= ds->GetCount())
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(), m_dataClass->GetName(),tr("Relative buffer index out of bounds: %2.  The valid range is 0 to %3.").arg(index).arg(ds->GetCount()-1));
      return false;
   }
   else
   {
      return true;
   }
}

bool DataSetSW::BoundsCheckLogical(double index)
{
   auto ds = static_cast<DataSet*>(m_dataClass);
   uint64_t end = ds->GetFirstIndex() + ds->GetCount();
   if (index < ds->GetFirstIndex() || index >= end)
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(), m_dataClass->GetName(),tr("Logical buffer index out of bounds: %2.  The valid range is %3 to %4.").arg(index).arg(ds->GetFirstIndex()).arg(end-1));
      return false;
   }
   else
   {
      return true;
   }
}



}
