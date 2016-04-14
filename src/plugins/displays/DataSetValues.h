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

#include "tools/TerbitDefs.h"
#include "connector-core/Block.h"

namespace terbit
{

class DataSetValuesView;
class DataSet;

static const char* DATASETVALUES_TYPENAME = "dataset-values";

class DataSetValues : public Block
{
   Q_OBJECT
public:

   enum DisplayFormat
   {
      DISPLAY_FORMAT_VALUE = 0,
      DISPLAY_FORMAT_HEX = 16,
      DISPLAY_FORMAT_OCT = 8
   };

   DataSetValues();
   virtual ~DataSetValues();

   virtual bool Init();
   virtual bool InteractiveInit();
   virtual bool InteractiveFocus();

   void ShowView();

   QObject* CreateScriptWrapper(QJSEngine* se);

   virtual void ApplyInputDataClass(DataClass* dc);

   virtual void Refresh();

   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   virtual void GetDirectDependencies(std::list<DataClass *> &dependsOn);

   bool MoveToIndex(uint64_t index);

   bool SetDataSet(DataSet* buf);
   void SetDataType(TerbitDataType dataType);
   void SetFormat(DataSetValues::DisplayFormat format);
   void SetColumns(uint64_t cols);
   void SetIndexFormat(DataSetValues::DisplayFormat indexFormat);

   bool RowForIndex(uint64_t index, uint64_t& row);
   void UpdateVisibleRows(uint64_t rowStart, uint64_t rowCount);
   uint64_t GetVisibleRowStart() { return m_visibleRowStart; }
   uint64_t GetVisibleRowCount() { return m_visibleRowCount; }

   TerbitDataType GetDataType() { return m_dataType; }
   DataSetValues::DisplayFormat GetFormat() { return m_format; }
   uint64_t GetColumns() { return m_cols; }
   uint64_t GetRows() { return m_rows; }

   DataSetValues::DisplayFormat GetIndexFormat() { return m_indexFormat; }
   bool CanRefreshData();

   bool StringToValue(uint64_t& value, const QString& text, DataSetValues::DisplayFormat textFormat);
   QString GetDisplayValue(uint64_t row, uint64_t col);
   QString GetDisplayRowIndex(uint64_t row);
   QString GetDisplayColumnIndex(uint64_t col);

   void DataUpdated(bool& rowsChanged);
   void LayoutUpdated();

   void RefreshData();
   void ParametersUpdated();

signals:
   void ModelNewData();
   void ModelStructureChanged();

private slots:
   void OnNewData(DataClass* bufferSource);
   void OnBeforeDataSetRemovedSlot(DataClass* dc);
   void OnDataSetSourceStructureChanged(DataSource* source);
   void OnDataSetSourceSet(DataSet* ds);
   void OnDataSetNameChanged(DataClass* dc);
   void OnViewClosed();

private:

   QString FormatValue(char* buf, TerbitDataType valueType, DataSetValues::DisplayFormat valueFormat, int valueSize) const;
   void UpdateVisibleRowsRequestData(uint64_t visibleBufferStart, uint64_t visibleBufferEnd);
   uint64_t CalculateRows();
   void AdjustVisibleRowStart();
   bool DisplayCellToSourceIndex(uint64_t row, uint64_t col, uint64_t &sourceIndex);
   char *CreateTempBuf(size_t byteCount);
   QString GenerateScriptForFormat(DisplayFormat format);

   DataSetValuesView* m_view;
   DataSet* m_dataSet;
   uint64_t m_rows, m_cols;
   DisplayFormat m_format;
   DisplayFormat m_indexFormat;
   TerbitDataType m_dataType;

   bool m_requestedData;
   uint64_t m_requestedBufferStart;
   size_t m_requestedBufferCount;
   uint64_t m_visibleRowStart, m_visibleRowCount, m_firstVisibleElement;
   char* m_tempBuf;
   size_t m_tempBufCount;

};

ScriptDocumentation* BuildScriptDocumentationDataSetValuesDisplay();

class DataSetValuesSW : public BlockSW
{
   Q_OBJECT
public:
   DataSetValuesSW(QJSEngine* se, DataSetValues* dsv);

   Q_INVOKABLE void SetDataType(int dataType);
   Q_INVOKABLE void SetFormat(int format);
   Q_INVOKABLE void SetColumns(int cols);
   Q_INVOKABLE void SetIndexFormat(int indexFormat);
   Q_INVOKABLE void SetDataSet(const QJSValue& valueDS);

   Q_PROPERTY(QJSValue FORMAT_VALUE READ GetFORMAT_VALUE)
   QJSValue GetFORMAT_VALUE() { return DataSetValues::DISPLAY_FORMAT_VALUE; }

   Q_PROPERTY(QJSValue FORMAT_HEX READ GetFORMAT_HEX)
   QJSValue GetFORMAT_HEX() { return DataSetValues::DISPLAY_FORMAT_HEX; }

   Q_PROPERTY(QJSValue FORMAT_OCTAL READ GetFORMAT_OCTAL)
   QJSValue GetFORMAT_OCTAL() { return DataSetValues::DISPLAY_FORMAT_OCT; }

private:
   DataSetValues* m_values;
};


}
