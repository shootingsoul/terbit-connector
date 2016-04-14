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

#include <map>
#include <list>
#include <QString>
#include <QObject>
#include "tools/TerbitDefs.h"
#include "plugins/scripting/ScriptBuilder.h"
#include "tools/Script.h"

namespace terbit
{

class Workspace;
class DataClassType;
class ScriptDocumentation;

class DataClass : public QObject
{
   Q_OBJECT

   friend class DataClassManager;
   friend class Block; //to set outputId only from addoutput on block
public:

   DataClass();
   virtual ~DataClass();

   DataClass* GetOwner() const { return m_owner; }
   void SetOwner(DataClass* owner);

   bool GetPublicScope() const { return m_publicScope; }

   virtual bool IsDataSet() const;
   virtual bool IsBlock() const;

   const QString& GetName() const { return m_name; }
   virtual void SetName(const QString& name);
   const QString& GetUniqueId() { return m_uniqueId; }
   void SetUniqueId(const QString& uniqueId);

   DataClassAutoId_t GetAutoId(void) { return m_autoId; }
   const DataClassType* GetType(void) const { return m_type; }

   BlockIOCategory_t GetOutputCategory() const { return m_outputCategory; }


   Workspace* GetWorkspace() { return m_workspace; }

   virtual QObject* CreateScriptWrapper(QJSEngine* se);
   virtual void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   virtual bool Init() { return true; }
   virtual bool InteractiveInit();
   virtual bool InteractiveFocus();
   // This function is called by the dataclass manager before removing
   // the dataclass instance (via delete).  The subclass MUST NOT delete
   // themselves in this function.
   virtual bool ImpendingDeletion();

   virtual void GetDirectDependencies(std::list<DataClass*>& dependsOn);

   virtual bool ShowPropertiesView();
   virtual void ClosePropertiesView();

signals:
   void NameChanged(DataClass* dc);
   void BeforeDeletion(DataClass* dc);
   void NewData(DataClass* dc);

protected slots:
   virtual void OnBeforeDeleteOwner(DataClass* dc);

private:
   DataClass(const DataClass& o); //disable copy ctor

   DataClass* m_owner;
   DataClassAutoId_t m_autoId;         // general purpose ID, returned from workspace (key from map)
   BlockIOCategory_t m_outputCategory;
   const DataClassType* m_type;

   bool m_publicScope;

   QString m_name;            // Name given by data set source/creator (may different than displayed on plot)
   QString m_uniqueId;

   Workspace* m_workspace;
};

typedef std::map<DataClassAutoId_t, DataClass*> DataClassMap;
typedef std::map<BlockIOCategory_t, DataClass*> DataClassOutputMap; //TODO *** remove

ScriptDocumentation* BuildScriptDocumentationDataClass();

class DataClassSW : public QObject
{
   Q_OBJECT
public:
   DataClassSW(QJSEngine* se, DataClass* dataClass);
   virtual ~DataClassSW() {}

   Q_INVOKABLE QJSValue GetOwner();
   Q_INVOKABLE bool IsDataSet();
   Q_INVOKABLE QJSValue GetType();

   Q_INVOKABLE void SetName(const QString& name);
   Q_INVOKABLE QString GetName();
   Q_INVOKABLE void SetUniqueId(const QString& uniqueId);
   Q_INVOKABLE QString GetUniqueId();

   Q_INVOKABLE virtual void ClosePropertiesWindow();
   Q_INVOKABLE virtual void ShowPropertiesWindow();

   DataClass* GetDataClass() { return m_dataClass; }

signals:
   void NewData();

private slots:
   void OnBeforeDeletion(DataClass*);

protected:
   QJSEngine* m_scriptEngine;
   DataClass* m_dataClass;
};
}
