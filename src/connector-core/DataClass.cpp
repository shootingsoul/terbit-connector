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

#include "DataClass.h"
#include "tools/Tools.h"
#include "Workspace.h"
#include "LogDL.h"


namespace terbit
{

DataClass::DataClass() : m_owner(NULL), m_autoId(INVALID_ID_VAL), m_outputCategory(INVALID_ID_VAL), m_type(NULL), m_publicScope(true), m_workspace(NULL)
{
}

DataClass::~DataClass()
{
}

void DataClass::SetOwner(DataClass *owner)
{
   if (m_owner)
   {
      disconnect(m_owner,SIGNAL(BeforeDeletion(DataClass*)),this, SLOT(OnBeforeDeleteOwner(DataClass*)));
   }
   m_owner = owner;
   if (m_owner)
   {
      connect(m_owner,SIGNAL(BeforeDeletion(DataClass*)),this, SLOT(OnBeforeDeleteOwner(DataClass*)));
   }
}

bool DataClass::IsDataSet() const
{
   return false;
}

bool DataClass::IsBlock() const
{
   return false;
}


void DataClass::SetName(const QString& name)
{
   m_name = name;
   emit NameChanged(this);
}

void DataClass::SetUniqueId(const QString &uniqueId)
{
   if (m_uniqueId.length() > 0)
   {
      if (m_workspace->GetDataClassManager().ReplaceUniqueId(m_uniqueId, uniqueId))
      {
         m_uniqueId = uniqueId;
      }
   }
   else
   {
      m_uniqueId = uniqueId; //outside of data class manager . . .
   }
}

bool DataClass::ImpendingDeletion()
{
   return true;
}

void DataClass::GetDirectDependencies(std::list<DataClass *> &dependsOn)
{
   dependsOn;
}

bool DataClass::InteractiveInit()
{
   switch (m_type->GetKind())
   {
   case DATA_CLASS_KIND_PROCESSOR:
   case DATA_CLASS_KIND_DEVICE:
      return ShowPropertiesView();
   default:
      return true;
   }
}

bool DataClass::InteractiveFocus()
{
   switch (m_type->GetKind())
   {
   case DATA_CLASS_KIND_PROCESSOR:
   case DATA_CLASS_KIND_DEVICE:
      return ShowPropertiesView();
   default:
      return true;
   }
}

bool DataClass::ShowPropertiesView()
{
   return true;
}

void DataClass::ClosePropertiesView()
{

}

void DataClass::OnBeforeDeleteOwner(DataClass *dc)
{
   dc;
   m_owner = NULL;
}

QObject *DataClass::CreateScriptWrapper(QJSEngine *se)
{
   return new DataClassSW(se, this);
}

void DataClass::BuildRestoreScript(ScriptBuilder &script, const QString &variableName)
{
   script; variableName;
}


ScriptDocumentation *BuildScriptDocumentationDataClass()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->AddScriptlet(new Scriptlet(QObject::tr("NewData"),"NewData.connect(OnNewData);",QObject::tr("Connect function to the new data event")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetOwner"),"GetOwner();",QObject::tr("Returns the owner object or undefined.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("IsDataSet"),"IsDataSet();",QObject::tr("Returns boolean")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetType"),"GetType();",QObject::tr("Returns object with type information")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetName"),"SetName(name);",QObject::tr("Sets the name (string) of the object.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetName"),"GetName();",QObject::tr("Returns the string name of the object.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetUniqueId"),"SetUniqueId(id);",QObject::tr("Sets the id (string) of the object.  The id must be unique across all instances.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetUniqueId"),"GetUniqueId();",QObject::tr("Returns the string id of the object.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ShowPropertiesWindow"),"ShowPropertiesWindow();",QObject::tr("Show the properties window for the object.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ClosePropertiesWindow"),"ClosePropertiesWindow();",QObject::tr("Close the properties window for the object.")));

   return d;
}

DataClassSW::DataClassSW(QJSEngine* se, DataClass *dataClass) : m_scriptEngine(se), m_dataClass(dataClass)
{
   connect(m_dataClass,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeDeletion(DataClass*)));
   connect(m_dataClass,SIGNAL(NewData(DataClass*)),this, SIGNAL(NewData()));
}

QJSValue DataClassSW::GetOwner()
{
   QJSValue res;
   if (m_dataClass->GetOwner())
   {
      res = m_scriptEngine->newQObject(m_dataClass->GetOwner()->CreateScriptWrapper(m_scriptEngine));
   }
   return res;
}

bool DataClassSW::IsDataSet()
{
   return m_dataClass->IsDataSet();
}

QJSValue DataClassSW::GetType()
{
   QJSValue res;

   res = m_scriptEngine->newQObject(m_dataClass->GetType()->CreateScriptWrapper(m_scriptEngine));

   return res;
}

void DataClassSW::SetName(const QString &name)
{
   m_dataClass->SetName(name);
}

QString DataClassSW::GetName()
{
   return m_dataClass->GetName();
}

void DataClassSW::SetUniqueId(const QString &uniqueId)
{
   m_dataClass->SetUniqueId(uniqueId);
}

QString DataClassSW::GetUniqueId()
{
   return m_dataClass->GetUniqueId();
}

void DataClassSW::OnBeforeDeletion(DataClass *)
{
   //if data class is being removed, then also remove it's script wrapper
   //this helps ensure that a data class script wrapper reference to a deleted object
   //will only crash the script and not entire program
   m_dataClass = NULL;
   delete this;
}

void DataClassSW::ClosePropertiesWindow()
{
   m_dataClass->ClosePropertiesView();
}

void DataClassSW::ShowPropertiesWindow()
{
   m_dataClass->ShowPropertiesView();
}

}
