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

#include <QString>
#include <QVariant>
#include <map>

#include "tools/TerbitDefs.h"
#include "tools/Tools.h"
#include "DataClass.h"

namespace terbit
{

class DataSet;
class Workspace;

typedef std::map<QString, QVariant> DataPropertiesMap;

class DataSource : public DataClass
{
   Q_OBJECT

public:
   DataSource();

   virtual bool InteractiveFocus();

   TerbitDataType GetDataType(void) const {return m_dataType;}
   uint64_t GetFirstIndex() const { return m_firstIndex; }
   uint64_t GetCount() const { return m_count; } //element count

   //i.e. can we call host->read(sourceId) In other words, is pull available?  It is possible to have data always pushed from source
   bool GetReadable() const { return m_readable; }
   void SetReadable(bool readable) { m_readable = readable; }

   //i.e. can we call host->write(sourceId)?
   bool GetWritable() const { return m_writable; }
   void SetWritable(bool writable) { m_writable = writable; }

   bool GetHasData() const { return m_hasData; }
   void SetHasData(bool hasData) { m_hasData = hasData; }

   size_t GetDefaultBufferElements() { return m_defaultBufferElements; }
   void SetDefaultBufferElements(size_t defaultBufferElements) { m_defaultBufferElements = defaultBufferElements; }

   const QString& GetDisplayViewTypeName() { return m_displayTypeName; }
   void SetDisplayViewTypeName(const QString& typeName) { m_displayTypeName = typeName; }
   virtual void ShowDisplayView();

   //helper function to test if settings are different and emit signal if needed
   void UpdateStructure(TerbitDataType dataType, uint64_t firstIndex, uint64_t count);

   DataPropertiesMap& GetProperties() { return m_properties; }
   QVariant GetPropertyValue(const QString& key);

   virtual void ReadRequest(uint64_t startIndex, size_t elementCount, DataClassAutoId_t bufferId) = 0;

   virtual QObject* CreateScriptWrapper(QJSEngine* se);

signals:
   void StructureChanged(DataSource* source); //index, element count, or data type change

protected:
   DataSource(const DataSource& o); //disable copy ctor

   QString m_displayTypeName;
   TerbitDataType m_dataType;
   uint64_t m_firstIndex;
   uint64_t m_count; //elements
   size_t m_defaultBufferElements;
   bool m_readable;
   bool m_writable;
   bool m_hasData;
   DataPropertiesMap m_properties;
};

ScriptDocumentation* BuildScriptDocumentationDataSource();

class DataSourceSW : public DataClassSW
{
   Q_OBJECT
public:
   DataSourceSW(QJSEngine* se, DataSource* source);

   Q_INVOKABLE QString GetDisplayTypeName();
   Q_INVOKABLE void SetDisplayTypeName(const QString& fullTypeName);
   Q_INVOKABLE void ShowDisplayWindow();

   Q_INVOKABLE double GetCount();
   Q_INVOKABLE int GetDataType();
   Q_INVOKABLE double GetFirstIndex();

   Q_INVOKABLE bool GetReadable();
   Q_INVOKABLE void SetReadable(bool readable);

   Q_INVOKABLE bool GetWritable();
   Q_INVOKABLE void SetWritable(bool writable);

   Q_INVOKABLE bool GetHasData();
   Q_INVOKABLE void SetHasData(bool hasData);
   Q_INVOKABLE void EmitNewData();

   Q_INVOKABLE double GetDefaultBufferElements();
   Q_INVOKABLE void SetDefaultBufferElements(double defaultBufferElements);
   Q_INVOKABLE void UpdateStructure(int dataType, double firstIndex, double count);
   Q_INVOKABLE QVariant GetPropertyValue(const QString& key);
   Q_INVOKABLE void SetPropertyValue(const QString& key, const QVariant& value);

   Q_INVOKABLE void ReadRequest(double startIndex, double elementCount, const QJSValue& destination);

};

}
