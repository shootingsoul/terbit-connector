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

#include <QCoreApplication>
#include "DataSetValues.h"
#include "DataSetValuesView.h"
#include "connector-core/DataSet.h"
#include "connector-core/Workspace.h"
#include "connector-core/LogDL.h"

namespace terbit
{

DataSetValues::DataSetValues() : Block(), m_dataSet(NULL), m_view(NULL), m_rows(0), m_cols(8),
   m_format(DataSetValues::DISPLAY_FORMAT_VALUE), m_indexFormat(DataSetValues::DISPLAY_FORMAT_VALUE), m_dataType(TERBIT_UINT8),
   m_requestedData(false), m_requestedBufferStart(0),m_requestedBufferCount(0),
   m_visibleRowStart(0), m_visibleRowCount(0), m_firstVisibleElement(0), m_tempBuf(NULL),m_tempBufCount(0)
{
}

DataSetValues::~DataSetValues()
{
   if (m_view)
   {
      disconnect(m_view,SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      delete m_view;
   }

   if (m_tempBuf != NULL)
   {
      free(m_tempBuf);
   }
}


bool DataSetValues::Init()
{
   ShowView();
   return true;
}

bool DataSetValues::InteractiveInit()
{
return true;
}

bool DataSetValues::InteractiveFocus()
{
   GetWorkspace()->WidgetAlert(m_view);
   return true;
}

void DataSetValues::ShowView()
{
   if (m_view)
   {
      m_view->raise();
   }
   else
   {
      m_view = new DataSetValuesView(this);
      GetWorkspace()->AddDockWidget(m_view);
      connect(m_view,SIGNAL(destroyed()),this, SLOT(OnViewClosed()));
      m_view->setFocus();
   }
}

void DataSetValues::OnViewClosed()
{
   m_view = NULL;
   //remove ourself
   GetWorkspace()->DeleteInstance(this->GetAutoId());
}

void DataSetValues::Refresh()
{
   //force full refresh with structure change
   emit ModelStructureChanged();
}

QObject *DataSetValues::CreateScriptWrapper(QJSEngine *se)
{
   return new DataSetValuesSW(se,this);
}

void DataSetValues::ApplyInputDataClass(DataClass* dc)
{
   if (dc && dc->IsDataSet())
   {
      Block::ApplyInputDataClass(dc);
      DataSet* buf = static_cast<DataSet*>(dc);
      if (SetDataSet(buf) == true)
      {
         Refresh();
         //try to load data by default if we can
         //do this after the parameters updated sets up all our rows/counts/etc.
         if (buf->GetHasData() && buf->CanRefreshData())
         {
            RefreshData();
         }
      }
   }
}

bool DataSetValues::SetDataSet(DataSet *ds)
{
   if (ds && !ds->GetReadable())
   {
      LogError2(GetType()->GetLogCategory(), GetName(),tr("Data Set Values Display can't be set to a data set that is not readable."));
      return false;
   }

   if (m_dataSet)
   {
      disconnect(m_dataSet,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeDataSetRemovedSlot(DataClass*)));
      disconnect(m_dataSet,SIGNAL(InputSourceAssigned(DataSet*)),this,SLOT(OnDataSetSourceSet(DataSet*)));
      disconnect(m_dataSet,SIGNAL(NewData(DataClass*)),this,SLOT(OnNewData(DataClass*)));
      disconnect(m_dataSet,SIGNAL(StructureChanged(DataSource*)),this,SLOT(OnDataSetSourceStructureChanged(DataSource*)));
      disconnect(m_dataSet,SIGNAL(NameChanged(DataClass*)),this,SLOT(OnDataSetNameChanged(DataClass*)));
   }
   m_dataSet = ds;
   if (m_dataSet)
   {
      connect(m_dataSet,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeDataSetRemovedSlot(DataClass*)));
      connect(m_dataSet,SIGNAL(InputSourceAssigned(DataSet*)),this,SLOT(OnDataSetSourceSet(DataSet*)));
      connect(m_dataSet,SIGNAL(NewData(DataClass*)),this,SLOT(OnNewData(DataClass*)));
      connect(m_dataSet,SIGNAL(StructureChanged(DataSource*)),this,SLOT(OnDataSetSourceStructureChanged(DataSource*)));
      connect(m_dataSet,SIGNAL(NameChanged(DataClass*)),this,SLOT(OnDataSetNameChanged(DataClass*)));

      m_dataType = ds->GetDataType();
      m_firstVisibleElement = ds->GetFirstIndex();
      OnDataSetNameChanged(m_dataSet);
   }

   ParametersUpdated();
   return true;
}

bool DataSetValues::MoveToIndex(uint64_t index)
{
   bool res = false;
   uint64_t row;
   if (RowForIndex(index, row))
   {
      //try to put move request in the center of the screen
      if (row >= m_visibleRowCount/2)
      {
         row -= m_visibleRowCount/2;
      }
      UpdateVisibleRows(row,m_visibleRowCount);
      res = true;
   }

   return res;
}

void DataSetValues::GetDirectDependencies(std::list<DataClass *> &dependsOn)
{
   if (m_dataSet)
   {
      dependsOn.push_back(m_dataSet);
   }
}

void DataSetValues::OnNewData(DataClass* source)
{
   source;
   //NOTE: already in GUI thread
   if (m_dataSet) //paranoid check
   {
      bool rowsChanged = false;
      uint64_t rows = CalculateRows();
      if (rows != m_rows)
      {
         rowsChanged = true;
         m_rows = rows;
         AdjustVisibleRowStart();
      }
      m_requestedData = false;

      if (rowsChanged)
      {
         //force full refresh
         emit ModelStructureChanged();
      }
      else
      {
         //only new data
         emit ModelNewData();
      }
   }
}

void DataSetValues::OnBeforeDataSetRemovedSlot(DataClass *dc)
{
   dc;
   //assume removing data set for model . . . only one we signed up for the event
   SetDataSet(NULL);
}

void DataSetValues::OnDataSetSourceSet(DataSet *buf)
{
   //new data source . . . changes the underlying model
   connect(&(buf->GetInputSource()),SIGNAL(StructureChanged(DataSource*)),this, SLOT(OnDataSetSourceStructureChanged(DataSource*)));
   ParametersUpdated();
}

void DataSetValues::OnDataSetSourceStructureChanged(DataSource* source)
{
   source;
   //may have new row count . . .
   //underlying model changed
   ParametersUpdated();
}


void DataSetValues::OnDataSetNameChanged(DataClass *dc)
{
   SetName(QString(tr("%1 Values")).arg(dc->GetName()));
}


template<typename DataType>
QString FormatValueTemplate(char* buf, DataSetValues::DisplayFormat valueFormat, int valueSize)
{
   QString res;

   DataType value = *((DataType*)buf);
   switch (valueFormat)
   {
   case DataSetValues::DISPLAY_FORMAT_VALUE:
      res = QString::number(value);
      break;
   default:
      {
         int format;
         if (valueFormat == DataSetValues::DISPLAY_FORMAT_OCT)
         {
            format = 8;
         }
         else
         {
            format = 16;
         }
         res = QString::number(value, format);
         int chars = valueSize * 8; //total bits
         if (valueFormat == DataSetValues::DISPLAY_FORMAT_HEX)
         {
            chars /= 4; //hex
         }
         else
         {
            chars = (int)ceil(chars/3.0); //ocatal
         }
         if (res.length() < chars)
         {
            chars -= res.length();
            while (chars > 0)
            {
               res = res.prepend('0');
               --chars;
            }
         }
         else if (res.length() > chars)
         {
            res = res.right(chars);
         }
      }
      break;
   }

   return res;
}


template<typename DataType>
QString FormatValueFloatDouble(char* buf)
{
   QString res;

   DataType value = *((DataType*)buf);
   res = QString::number(value);
   return res;
}

QString DataSetValues::FormatValue(char* buf, TerbitDataType valueType, DataSetValues::DisplayFormat valueFormat, int valueSize) const
{
   switch (valueType)
   {
   case TERBIT_INT8:
      return FormatValueTemplate<int8_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_INT16:
      return FormatValueTemplate<int16_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_INT32:
      return FormatValueTemplate<int32_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_INT64:
      return FormatValueTemplate<int64_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_UINT8:
      return FormatValueTemplate<uint8_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_UINT16:
      return FormatValueTemplate<uint16_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_UINT32:
      return FormatValueTemplate<uint32_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_UINT64:
      return FormatValueTemplate<uint64_t>(buf,valueFormat, valueSize);
      break;
   case TERBIT_FLOAT:
      return FormatValueFloatDouble<float>(buf);
      break;
   case TERBIT_DOUBLE:
      return FormatValueFloatDouble<double>(buf);
      break;
   };
   LogError2(GetType()->GetLogCategory(), GetName(),tr("Data Set Values can't format value %1.").arg(valueType));
   return QString("");

}

void DataSetValues::SetDataType(TerbitDataType dataType)
{
   m_dataType = dataType;
   ParametersUpdated();
}

void DataSetValues::SetFormat(DataSetValues::DisplayFormat format)
{
   m_format = format;
   ParametersUpdated();
}

void DataSetValues::SetColumns(uint64_t cols)
{
   m_cols = cols;
   ParametersUpdated();
}

void DataSetValues::SetIndexFormat(DataSetValues::DisplayFormat indexFormat)
{
   m_indexFormat = indexFormat;
   ParametersUpdated();
}

bool DataSetValues::RowForIndex(uint64_t elementIndex, uint64_t& row)
{
   bool res = false;

   if (m_rows > 0)
   {
      size_t valueSize = TerbitDataTypeSize(m_dataType);
      size_t bufferElementSize = TerbitDataTypeSize(m_dataSet->GetDataType());
      uint64_t start, end;
      start = m_dataSet->GetInputSource().GetFirstIndex();

      if (valueSize == bufferElementSize)
      {
         //logical index
         end = m_rows * m_cols;
         if (elementIndex >= start && elementIndex < end)
         {
            row = (elementIndex - start)/m_cols;
            res = true;
         }
      }
      else
      {
         //values display data type is a different size . . .
         //assume index represents byte address  . . .
         size_t rowSize = valueSize * m_cols;

         uint64_t address = elementIndex * bufferElementSize;
         end = start + m_rows * rowSize;

         if (address >= start && address < end)
         {
            row = (address - start)/rowSize;
            res = true;
         }
      }
   }

   return res;
}


bool DataSetValues::CanRefreshData()
{
   bool res;
   if (m_dataSet)
   {
      res = m_dataSet->CanRefreshData();
   }
   else
   {
      res = false;
   }
   return res;
}

void DataSetValues::RefreshData()
{
   //refresh . . .
   if (m_dataSet)
   {
      m_dataSet->RefreshData();
   }
}

void DataSetValues::UpdateVisibleRows(uint64_t rowStart, uint64_t rowCount)
{   
   m_visibleRowStart = rowStart;
   m_visibleRowCount = rowCount;
   AdjustVisibleRowStart();

   if (m_visibleRowCount > 0)
   {
      if (m_dataSet && m_dataSet->CanRefreshData())
      {         
         uint64_t compareStart;
         size_t compareCount;
         if (m_requestedData)
         {
            compareStart = m_requestedBufferStart;
            compareCount = m_requestedBufferCount;
         }
         else
         {
            compareStart = m_dataSet->GetFirstIndex();
            compareCount = m_dataSet->GetCount();
         }


         uint64_t visibleBufferStart;
         uint64_t visibleBufferEnd;         
         visibleBufferStart = m_firstVisibleElement;
         if (m_visibleRowStart+m_visibleRowCount >= m_rows)
         {
            visibleBufferEnd = m_dataSet->GetInputSource().GetCount()-1;  //at the end of the data display
         }
         else
         {
            DisplayCellToSourceIndex(m_visibleRowStart+m_visibleRowCount-1,m_cols-1,visibleBufferEnd);
         }

         //if near the edge then request data ahead of time, add extra elements to the visible area  . . .
         uint64_t elementThreshold = m_cols*10;
         if (visibleBufferStart - m_dataSet->GetInputSource().GetFirstIndex() > elementThreshold)
         {
            visibleBufferStart -= elementThreshold;
         }
         else
         {
            visibleBufferStart = m_dataSet->GetInputSource().GetFirstIndex();
         }
         if (m_dataSet->GetInputSource().GetFirstIndex() + m_dataSet->GetInputSource().GetCount() - visibleBufferEnd > elementThreshold)
         {
            visibleBufferEnd += elementThreshold;
         }
         else
         {
            visibleBufferEnd = m_dataSet->GetInputSource().GetFirstIndex() + m_dataSet->GetInputSource().GetCount() - 1;
         }

         //if the visible is out range then request the data
         if (visibleBufferStart < compareStart || visibleBufferEnd >= compareStart + compareCount)
         {
            UpdateVisibleRowsRequestData(visibleBufferStart, visibleBufferEnd);
         }
         else
         {
            emit ModelNewData(); //change in visible area of the data set for new data
         }
      }
      else
      {
         emit ModelNewData(); //change in visible area of the data set for new data
      }
   }
}

void DataSetValues::UpdateVisibleRowsRequestData(uint64_t visibleBufferStart, uint64_t visibleBufferEnd)
{

   m_requestedBufferCount = m_dataSet->GetInputSource().GetDefaultBufferElements();

   size_t visibleCount = visibleBufferEnd - visibleBufferStart + 1;

   //put the visible elements in middle of request
   if (m_requestedBufferCount >= visibleCount)
   {
      uint64_t slack = (m_requestedBufferCount - visibleCount) / 2;
      if (visibleBufferStart - m_dataSet->GetInputSource().GetFirstIndex() < slack)
      {
          m_requestedBufferStart = m_dataSet->GetInputSource().GetFirstIndex();
      }
      else
      {
         m_requestedBufferStart = visibleBufferStart - slack;
      }

      //check end boundary and push back the request if needed
      if (m_dataSet->GetInputSource().GetCount() - m_requestedBufferStart < m_requestedBufferCount)
      {
         m_requestedBufferStart = m_dataSet->GetInputSource().GetCount() - m_requestedBufferCount;
      }
   }
   else
   {
      //go backwards from the end
      m_requestedBufferStart = visibleBufferEnd + 1 - m_requestedBufferCount;

      //visible row is larger than buffer in the data set
      LogWarning(g_log.general, "The data set buffer is too small to display all values at once.");
   }

   DataSource* rds = &(m_dataSet->GetInputSource());

   m_dataSet->SetHasData(false); //clear that we have data while we setup buffer for read request
   m_dataSet->CreateBuffer(rds->GetDataType(), m_requestedBufferStart, m_requestedBufferCount);
   rds->ReadRequest(m_requestedBufferStart, m_requestedBufferCount, m_dataSet->GetAutoId());
   m_requestedData = true;
}


QString DataSetValues::GetDisplayValue(uint64_t row, uint64_t col)
{
   if (!m_dataSet || !m_dataSet->GetHasData())
   {
      return QString("");
   }

   uint64_t index;
   if (DisplayCellToSourceIndex(row,col,index))
   {
      size_t valueSize = TerbitDataTypeSize(m_dataType);
      size_t bufferElementSize = TerbitDataTypeSize(m_dataSet->GetDataType());
      size_t bufferStride = m_dataSet->GetStrideBytes();

      if (valueSize == bufferElementSize || (valueSize > bufferElementSize && bufferElementSize == bufferStride))
      {
         //same size or display is greater and data is contiguous
         size_t offset = (index - m_dataSet->GetFirstIndex()) * bufferStride;
         char* buf = (char*)m_dataSet->GetBufferAddress() + offset;
         return FormatValue(buf, m_dataType, m_format, valueSize);
      }
      else if (valueSize < bufferElementSize)
      {
         //find out where in the buffer element bytes we are interested in
         size_t displayByteStart = ((row*valueSize*m_cols+col*valueSize)- m_dataSet->GetInputSource().GetFirstIndex()) % bufferElementSize;
         size_t offset = (index - m_dataSet->GetFirstIndex()) * bufferStride + displayByteStart;

         char* buf = (char*)m_dataSet->GetBufferAddress() + offset;
         return FormatValue(buf, m_dataType, m_format, valueSize);
      }
      else
      {
         //display crosses over buffer elements and the buffer is not contiguous
         //piece it together

         //TODO: allocate temp buf ahead of time instead of lazy allocation

         size_t pieces = (size_t)ceil(valueSize/(double)bufferElementSize);
         char* tempBuf = CreateTempBuf(pieces*bufferElementSize);
         char* pieceBuf = tempBuf;

         size_t offset = (index - m_dataSet->GetFirstIndex()) * bufferStride;
         char* buf = (char*)m_dataSet->GetBufferAddress() + offset;

         for(size_t piece = 0; piece < pieces; ++piece, buf += bufferStride, pieceBuf += bufferElementSize)
         {
            memcpy(pieceBuf,buf,bufferElementSize);
         }

         QString res;
         res = FormatValue(tempBuf, m_dataType, m_format, valueSize);
         return res;
      }

   }
   else
   {
      return QString("");
   }
}

char* DataSetValues::CreateTempBuf(size_t byteCount)
{
   if (byteCount > m_tempBufCount)
   {
      if (m_tempBuf != NULL)
      {
         free(m_tempBuf);
      }
      m_tempBuf = (char*)malloc(byteCount);
      m_tempBufCount = byteCount;
   }
   return m_tempBuf;
}

QString DataSetValues::GetDisplayRowIndex(uint64_t row)
{
   if (!m_dataSet || !m_dataSet->GetHasData())
   {
       return QString("");
   }

   uint64_t index;
   if (DisplayCellToSourceIndex(row,0,index))
   {
      return QString(FormatValueTemplate<uint64_t>((char*)&index,m_indexFormat,sizeof(uint64_t)));
   }
   else
   {
      return QString("");
   }
}

QString DataSetValues::GetDisplayColumnIndex(uint64_t col)
{
   if (m_cols < 255)
   {
      uint8_t i = (uint8_t)col;
      return QString(FormatValueTemplate<uint8_t>((char*)&i,m_indexFormat,sizeof(uint8_t)));
   }
   else
   {
      return QString(FormatValueTemplate<uint64_t>((char*)&col,m_indexFormat,sizeof(uint64_t)));
   }
}


bool DataSetValues::DisplayCellToSourceIndex(uint64_t row, uint64_t col, uint64_t& sourceIndex)
{
   bool res = false;

   if (row < m_rows)
   {
      size_t valueSize = TerbitDataTypeSize(m_dataType);
      size_t bufferElementSize = TerbitDataTypeSize(m_dataSet->GetDataType());

      if (valueSize == bufferElementSize)
      {
         sourceIndex =  row * m_cols;
         if (col == 0)
         {
            sourceIndex += m_dataSet->GetInputSource().GetFirstIndex();
            res = true;
         }
         else if (m_dataSet->GetInputSource().GetCount() - sourceIndex > col)
         {
            sourceIndex += col + m_dataSet->GetInputSource().GetFirstIndex();
            res = true;
         }
      }
      else
      {
         //display size differs from  buffer
         //convert to bytes (common denominator) and figure out which element/index it refers to
         size_t rowSize = valueSize * m_cols;
         sourceIndex = (row*rowSize) / bufferElementSize;

         size_t colCount = (valueSize*col)/bufferElementSize;
         if (colCount == 0)
         {
            sourceIndex += m_dataSet->GetInputSource().GetFirstIndex();
            res = true;
         }
         else if (m_dataSet->GetInputSource().GetCount() - sourceIndex > colCount)
         {
            sourceIndex += colCount + m_dataSet->GetInputSource().GetFirstIndex();
            res = true;
         }
      }
   }

   return res;
}

void DataSetValues::AdjustVisibleRowStart()
{
   if (m_rows <= m_visibleRowCount)
   {
      m_visibleRowStart = 0;
   }
   else if (m_visibleRowStart > m_rows - m_visibleRowCount)
   {
      m_visibleRowStart = m_rows - m_visibleRowCount;
   }
   if (m_dataSet)
   {
      DisplayCellToSourceIndex(m_visibleRowStart,0,m_firstVisibleElement);
   }
}

uint64_t DataSetValues::CalculateRows()
{
   uint64_t rows = 0;

   if (m_dataSet)
   {
      size_t valueSize = TerbitDataTypeSize(m_dataType);      
      size_t bufferElementSize = TerbitDataTypeSize(m_dataSet->GetDataType());

      if (valueSize == bufferElementSize)
      {
         //sizes are the same, use logicla elements to determine row count
         //this allows us to represent more logical data
         rows = (uint64_t)ceil(m_dataSet->GetInputSource().GetCount() / (long double)m_cols);
      }
      else
      {
         //sizes are different between what we're displaying and the underlying source
         //convert to bytes (common denominator) to determine total rows
         size_t rowSize = valueSize * m_cols;
         rows = (uint64_t)ceil((m_dataSet->GetInputSource().GetCount() * bufferElementSize) / (long double)rowSize);
      }
   }
   return rows;
}


//TODO: use TerbitValue and support other result types aside from uint64
//perhaps move this to tools . . .
bool DataSetValues::StringToValue(uint64_t& value, const QString &text, DataSetValues::DisplayFormat textFormat)
{
   bool res = false;
   switch (textFormat)
   {
   case DataSetValues::DISPLAY_FORMAT_HEX:
     value = text.toULongLong(&res,16);
     break;
   case DataSetValues::DISPLAY_FORMAT_OCT:
      value = text.toULongLong(&res,8);
      break;
   default:
      value = text.toULongLong(&res);
      break;
   }
   return res;
}

void DataSetValues::ParametersUpdated()
{
   m_rows = CalculateRows();
   //try to keep the same visible position
   RowForIndex(m_firstVisibleElement, m_visibleRowStart);
   AdjustVisibleRowStart();

   emit ModelStructureChanged();
}

QString DataSetValues::GenerateScriptForFormat(DisplayFormat format)
{
   QString res;
   switch (format)
   {
   case DISPLAY_FORMAT_VALUE:
      res = "FORMAT_VALUE";
      break;
   case DISPLAY_FORMAT_HEX:
      res = "FORMAT_HEX";
      break;
   case DISPLAY_FORMAT_OCT:
      res = "FORMAT_OCTAL";
      break;
   }

   return res;
}

void DataSetValues::BuildRestoreScript(ScriptBuilder &script, const QString &variableName)
{
   if (m_dataSet)
   {
      script.add(QString("%1.SetDataSet(%2);").arg(variableName).arg(ScriptEncode(m_dataSet->GetUniqueId())));
   }
   script.add(QString("%1.SetDataType(%2);").arg(variableName).arg(ScriptBuilder::GenerateScript(GetDataType())));
   script.add(QString("%1.SetFormat(%1.%2);").arg(variableName).arg(GenerateScriptForFormat(GetFormat())));
   script.add(QString("%1.SetColumns(%2);").arg(variableName).arg(GetColumns()));
   script.add(QString("%1.SetIndexFormat(%1.%2);").arg(variableName).arg(GenerateScriptForFormat(GetIndexFormat())));
   script.add(QString("%1.Refresh();").arg(variableName));
}

ScriptDocumentation *BuildScriptDocumentationDataSetValuesDisplay()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("Displays the values for a data set in a table."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetDataSet"), "SetDataSet(ds);",QObject::tr("Sets the data set to display.  The ds variable may be a reference or unique id string.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetFormat"), "SetFormat(format);",QObject::tr("Set display format enum.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetDataType"), "SetDataType(dataType);",QObject::tr("Display in the specified data type (terbit enum) instead of the native data set data type.  For examply, the data set may be byte addressable and you want to display uint32.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetColumns"), "SetColumns(cols);",QObject::tr("The number of columns to display.  A value for each data type is in a column.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetIndexFormat"), "SetIndexFormat(format);",QObject::tr("Set the index display format enum.")));

   ScriptDocumentation* f = new ScriptDocumentation();
   f->SetName(QObject::tr("Formats"));
   f->SetSummary(QObject::tr("Display formats"));
   f->AddScriptlet(new Scriptlet(QObject::tr("Value"), "FORMAT_VALUE",QObject::tr("Native value format.")));
   f->AddScriptlet(new Scriptlet(QObject::tr("Hex"), "FORMAT_HEX",QObject::tr("Hexidecimal format.")));
   f->AddScriptlet(new Scriptlet(QObject::tr("Octal"), "FORMAT_OCTAL",QObject::tr("Octal format.")));

   d->AddSubDocumentation(f);

   return d;
}


DataSetValuesSW::DataSetValuesSW(QJSEngine *se, DataSetValues *dsv) : BlockSW(se,dsv), m_values(dsv)
{
}

void DataSetValuesSW::SetDataType(int dataType)
{
   m_values->SetDataType((TerbitDataType)dataType);
}

void DataSetValuesSW::SetFormat(int format)
{
   m_values->SetFormat((DataSetValues::DisplayFormat)format);
}

void DataSetValuesSW::SetColumns(int cols)
{
   m_values->SetColumns(cols);
}

void DataSetValuesSW::SetIndexFormat(int indexFormat)
{
   m_values->SetIndexFormat((DataSetValues::DisplayFormat)indexFormat);
}

void DataSetValuesSW::SetDataSet(const QJSValue& valueDS)
{
   DataClass* dc = m_values->GetWorkspace()->FindInstance(valueDS);
   if (dc && dc->IsDataSet())
   {
      m_values->SetDataSet(static_cast<DataSet*>(dc));
   }
   else
   {
      LogError2(m_values->GetType()->GetLogCategory(), m_values->GetName(),tr("Script Data Set Values Display SetDataSet invalid argument"));
   }
}


}
