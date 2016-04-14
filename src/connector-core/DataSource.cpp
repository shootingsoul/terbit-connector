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

#include "DataSource.h"
#include "DataSet.h"
#include "LogDL.h"
#include "Workspace.h"
#include "Block.h"

namespace terbit
{

DataSource::DataSource() : DataClass(), m_dataType(TERBIT_UINT8), m_firstIndex(0), m_count(0), m_defaultBufferElements(0),
                                 m_readable(false), m_writable(false), m_hasData(false)
{
}


bool DataSource::InteractiveFocus()
{
   if (GetDisplayViewTypeName().length() > 0)
   {
      DataClass* display = GetWorkspace()->CreateInstance(GetDisplayViewTypeName(),NULL,true);
      display->InteractiveInit();
      if (display->IsBlock())
      {
         DataSet* buf = CreateRemoteDataSet(this, NULL,true);
         if (buf)
         {
            buf->InteractiveInit();
            static_cast<Block*>(display)->ApplyInputDataClass(buf);
         }
      }
   }
   return true;
}


void DataSource::UpdateStructure(TerbitDataType dataType, uint64_t firstIndex, uint64_t count)
{
   if (dataType != m_dataType || firstIndex != m_firstIndex || count != m_count)
   {
      m_dataType = dataType;
      m_firstIndex = firstIndex;
      m_count = count;

      emit StructureChanged(this);
   }
}

QVariant DataSource::GetPropertyValue(const QString &key)
{
   //returns invalid qvariant if key not found
   QVariant res;

   DataPropertiesMap::iterator i = m_properties.find(key);
   if (i != m_properties.end())
   {
      res = i->second;
   }

   return res;
}

void DataSource::ShowDisplayView()
{
   if (GetDisplayViewTypeName().length() > 0)
   {
      DataClass* dc = GetWorkspace()->CreateInstance(GetDisplayViewTypeName(), NULL);
      if (dc && dc->GetType()->GetKind() == DATA_CLASS_KIND_DISPLAY)
      {
         Block* d = static_cast<Block*>(dc);
         d->InteractiveInit();
         d->SetName(GetName());
         DataSet* ds = CreateRemoteDataSet(this, NULL,true);
         if (ds)
         {
            ds->InteractiveInit();
            d->ApplyInputDataClass(ds);
         }
      }
      else
      {
         LogError2(GetType()->GetLogCategory(), GetName(),tr("Failed to create display of type '%1'").arg(GetDisplayViewTypeName()));
      }
   }
}

QObject *DataSource::CreateScriptWrapper(QJSEngine *se)
{
   return new DataSourceSW(se,this);
}

ScriptDocumentation *BuildScriptDocumentationDataSource()
{
   ScriptDocumentation* d = BuildScriptDocumentationDataClass();

   d->AddScriptlet(new Scriptlet(QObject::tr("GetDisplayTypeName"), "GetDisplayTypeName();",QObject::tr("Default display full type name string.")));
   d->AddScriptlet(new Scriptlet("SetDisplayTypeName", "SetDisplayTypeName(fullTypeName);","Set default display full type name string."));
   d->AddScriptlet(new Scriptlet("ShowDisplayWindow", "ShowDisplayWindow();","Show the default display."));

   d->AddScriptlet(new Scriptlet("GetCount", "GetCount();","The number of elements."));
   d->AddScriptlet(new Scriptlet("GetDataType", "GetDataType();","The element data type (enum)."));
   d->AddScriptlet(new Scriptlet("GetFirstIndex", "GetFirstIndex();","The logical index of the first element.  The default is 0."));

   d->AddScriptlet(new Scriptlet("GetReadable", "GetReadable();","Permission to read (boolean)."));
   d->AddScriptlet(new Scriptlet("SetReadable", "SetReadable(readable);","Set permission to read (boolean)."));

   d->AddScriptlet(new Scriptlet("GetWritable", "GetWritable();","Permission to write (boolean)."));
   d->AddScriptlet(new Scriptlet("SetWritable", "SetWritable(writable);","Set permission to write (boolean)."));

   d->AddScriptlet(new Scriptlet("GetHasData", "GetHasData();","Contains valid data (boolean)."));
   d->AddScriptlet(new Scriptlet("SetHasData", "SetHasData(hasData);","Set contains valid data (boolean).  Set this after manually writing to the buffer."));

   d->AddScriptlet(new Scriptlet("EmitNewData", "EmitNewData();","Notify there is new data to all slots connected the new data signal."));

   d->AddScriptlet(new Scriptlet("GetDefaultBufferElements", "GetDefaultBufferElements();","The default number of buffer elements.  Used when trying to read from a source."));
   d->AddScriptlet(new Scriptlet("SetDefaultBufferElements", "SetDefaultBufferElements(defaultBufferElements);","The default number of buffer elements.  Used when trying to read from a source."));

   d->AddScriptlet(new Scriptlet("UpdateStructure", "UpdateStructure(dataType, firstIndex, count);","Set the data type (enum), first index and element count."));

   d->AddScriptlet(new Scriptlet("GetPropertyValue", "GetPropertyValue(key);","Get property value for the string key."));
   d->AddScriptlet(new Scriptlet("SetPropertyValue", "SetPropertyValue(key, value);","Set property value for the string key."));

   d->AddScriptlet(new Scriptlet("ReadRequest", "ReadRequest(startIndex, elementCount, destination);","Read into a destination data set.  The destination may be a data set reference or the unique id string of a data set."));

   return d;
}

DataSourceSW::DataSourceSW(QJSEngine *se, DataSource *source) : DataClassSW(se,source)
{

}

QString DataSourceSW::GetDisplayTypeName()
{
   return static_cast<DataSource*>(m_dataClass)->GetDisplayViewTypeName();
}

void DataSourceSW::SetDisplayTypeName(const QString &fullTypeName)
{
   static_cast<DataSource*>(m_dataClass)->SetDisplayViewTypeName(fullTypeName);
}

void DataSourceSW::ShowDisplayWindow()
{
   auto ds = static_cast<DataSource*>(m_dataClass);
   ds->ShowDisplayView();
}

double DataSourceSW::GetCount()
{
   return static_cast<DataSource*>(m_dataClass)->GetCount();
}

int DataSourceSW::GetDataType()
{
   return (int)(static_cast<DataSource*>(m_dataClass)->GetDataType());
}

double DataSourceSW::GetFirstIndex()
{
   return static_cast<DataSource*>(m_dataClass)->GetFirstIndex();
}

bool DataSourceSW::GetReadable()
{
   return static_cast<DataSource*>(m_dataClass)->GetReadable();
}

void DataSourceSW::SetReadable(bool readable)
{
   static_cast<DataSource*>(m_dataClass)->SetReadable(readable);
}

bool DataSourceSW::GetWritable()
{
   return static_cast<DataSource*>(m_dataClass)->GetWritable();
}

void DataSourceSW::SetWritable(bool writable)
{
   static_cast<DataSource*>(m_dataClass)->SetWritable(writable);
}

bool DataSourceSW::GetHasData()
{
   return static_cast<DataSource*>(m_dataClass)->GetHasData();
}

void DataSourceSW::SetHasData(bool hasData)
{
   static_cast<DataSource*>(m_dataClass)->SetHasData(hasData);
}

void DataSourceSW::EmitNewData()
{
   auto ds = static_cast<DataSource*>(m_dataClass);
   ds->SetHasData(true);
   emit ds->NewData(ds);
}

double DataSourceSW::GetDefaultBufferElements()
{
   return static_cast<DataSource*>(m_dataClass)->GetDefaultBufferElements();
}

void DataSourceSW::SetDefaultBufferElements(double defaultBufferElements)
{
   static_cast<DataSource*>(m_dataClass)->SetDefaultBufferElements(defaultBufferElements);
}

void DataSourceSW::UpdateStructure(int dataType, double firstIndex, double count)
{
   static_cast<DataSource*>(m_dataClass)->UpdateStructure((TerbitDataType)dataType,firstIndex,count);
}

QVariant DataSourceSW::GetPropertyValue(const QString &key)
{
   return static_cast<DataSource*>(m_dataClass)->GetPropertyValue(key);
}

void DataSourceSW::SetPropertyValue(const QString& key, const QVariant &value)
{
   static_cast<DataSource*>(m_dataClass)->GetProperties()[key] = value;
}

void DataSourceSW::ReadRequest(double startIndex, double elementCount, const QJSValue& destination)
{
   auto dest = m_dataClass->GetWorkspace()->FindInstance(destination);
   if (dest)
   {
      static_cast<DataSource*>(m_dataClass)->ReadRequest(startIndex,elementCount,dest->GetAutoId());
   }
   else
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(), m_dataClass->GetName(),tr("ReadRequest destination not found"));
   }
}

}
