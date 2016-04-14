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


#include <QList>
#include <QObject>
#include "DataClass.h"
#include "BlockIOContainer.h"

namespace terbit
{

class DataSource;
class DataSet;

class Block : public DataClass
{
   Q_OBJECT
public:
   Block();
   virtual ~Block();

   virtual bool IsBlock() const;

   void ApplyInput(DataClassAutoId_t id);
   virtual void ApplyInputDataClass(DataClass* dc);

   BlockIOContainer& GetInputs() { return m_inputs; }
   BlockIOContainer& GetOutputs() { return m_outputs; }

   virtual void Refresh() {}

signals:
   void OutputAdded(Block* block, DataClass* output);

protected:
   bool AddOutput(BlockIOCategory_t category, DataClass* output);
   bool RemoveOutput(BlockIOCategory_t category);

private:
   Block(const Block& o); //disable copy ctor

   BlockIOContainer m_inputs;
   BlockIOContainer m_outputs;
};

ScriptDocumentation* BuildScriptDocumentationBlock();

class BlockSW : public DataClassSW
{
   Q_OBJECT
public:
   BlockSW(QJSEngine* se, Block* block);
   virtual ~BlockSW() {}

   Q_INVOKABLE void ApplyInputDataClass(const QJSValue& input);
   Q_INVOKABLE QJSValue GetOutputs();
   Q_INVOKABLE QJSValue GetInputs();
   Q_INVOKABLE void Refresh();
};

}
