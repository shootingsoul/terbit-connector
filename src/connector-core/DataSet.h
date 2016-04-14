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

#include <stdint.h>
#include <tools/Tools.h>
#include <tools/TerbitValue.h>
#include "DataSource.h"

namespace terbit
{

static const char* DATASET_TYPENAME = "dataset";

class DataSet : public DataSource
{
   Q_OBJECT
public:
   DataSet();
   virtual ~DataSet();

   bool ImpendingDeletion();

   bool IsRemote();
   virtual bool IsDataSet() const;
   virtual bool Init();
   virtual bool InteractiveInit();
   virtual bool InteractiveFocus();

   void SetBuffer(TerbitDataType type, uint64_t firstIndex, size_t elementCount, void* bufferAddress, size_t strideBytes);
   void CreateBuffer(TerbitDataType type, uint64_t firstIndex, size_t elementCount);
   void CreateBufferAsIndex(uint64_t firstIndex, size_t elementCount);

   void* GetBufferAddress(void) const {return m_buffer;}
   size_t GetStrideBytes(void) const {return m_strideBytes;}
   size_t GetAllocatedByteCount() const { return m_managedBufferSize; }

   virtual void ReadRequest(uint64_t startIndex, size_t elementCount, DataClassAutoId_t bufferId);

   void CalculateMinMax(TerbitValue& min, TerbitValue& max) const;
   bool ClosestIndex(const TerbitValue& key, size_t& index) const;
   bool BoundingIndicies(double startValue, double endValue, size_t& start, size_t& end) const;

   //TODO: make template, use TerbitValue or ideally get rid of this function
   double GetValueAtIndex(size_t index) const;
   void SetValueAtIndex(size_t index, double value);
   double GetValueAtLogicalIndex(uint64_t index) const;
   void SetValueAtLogicalIndex(uint64_t index, double value);


   DataSource& GetInputSource() { return *m_inputSource; }
   DataSet* GetIndexDataSet() { return m_indexDataSet; }
   void SetIndexDataSet(DataSet* indexBuffer);

   void ShowValuesView();
   virtual void ShowDisplayView();

   virtual QObject* CreateScriptWrapper(QJSEngine* se);

   bool CanRefreshData();
   void RefreshData();

   void SetInputSource(DataSource* source);

   virtual void GetDirectDependencies(std::list<DataClass *> &dependsOn);

signals:
   void InputSourceAssigned(DataSet* ds);
   void IndexAssigned(DataSet* ds);

private slots:
   void OnInputSourceNewData(DataClass* source);
   void OnBeforeInputSourceRemoved(DataClass* source);
   void OnBeforeIndexRemoved(DataClass* idx);
   void OnInputSourceStructureChanged(DataSource* source);


private:
   DataSet(const DataSet& o); //disable copy ctor

   void*  m_buffer;
   size_t m_strideBytes;
   size_t m_managedBufferSize;
   bool m_managedBuffer;
   DataSource* m_inputSource; //source for this data set
   DataSet* m_indexDataSet;
};

DataSet* CreateRemoteDataSet(DataSource* source, DataClass *owner, bool publicScope);

//TODO: generalize the type using TerbitValue or void pointers with TerbitDataType parameter
bool ClosestDataPoint(const DataSet* X, const DataSet* Y, double dataX, double& pointX, double& pointY);

ScriptDocumentation* BuildScriptDocumentationDataSet();

class DataSetSW : public DataSourceSW
{
   Q_OBJECT
public:
   DataSetSW(QJSEngine* se, DataSet* ds);

   DataSet* GetDataSet() { return static_cast<DataSet*>(m_dataClass); }

   Q_INVOKABLE void CreateBuffer(int dataType, double firstIndex, double elementCount);

   Q_INVOKABLE QJSValue GetIndexDataSet();
   Q_INVOKABLE void SetIndexDataSet(const QJSValue& ds);

   Q_INVOKABLE QJSValue GetValue(double index);
   Q_INVOKABLE void SetValue(double index, double value);
   Q_INVOKABLE QJSValue GetValueLogicalIndex(double index);
   Q_INVOKABLE void SetValueLogicalIndex(double index, double value);

   Q_INVOKABLE QJSValue CalculateMinMax();

   Q_INVOKABLE QJSValue GetInputDataSource();
   Q_INVOKABLE void SetInputDataSource(const QJSValue& source);
   Q_INVOKABLE bool IsRemote();

   Q_INVOKABLE bool CanRefreshData();
   Q_INVOKABLE void RefreshData();
   Q_INVOKABLE void ShowValuesWindow();

private:
   bool BoundsCheck(double index);
   bool BoundsCheckLogical(double index);


};

}
