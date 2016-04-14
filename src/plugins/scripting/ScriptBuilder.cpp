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
#include "ScriptBuilder.h"
#include "connector-core/DataClass.h"
#include "connector-core/DataSet.h"
#include "connector-core/Block.h"
#include "connector-core/DataClassManager.h"
#include "connector-core/DataClassType.h"
#include "connector-core/ScriptDocumentation.h"
#include "connector-core/Workspace.h"
#include <tools/Script.h>

namespace terbit
{

ScriptBuilder::ScriptBuilder()
{

}

const QString ScriptBuilder::VARIABLE_WORKSPACE = "terbit.workspace";
const QString ScriptBuilder::VARIABLE_TERBIT = "terbit";

void ScriptBuilder::add(const QString &line)
{
   m_script = m_script + line + "\n";
}

QString ScriptBuilder::ToScriptString()
{
   return m_script;
}

QString ScriptBuilder::GenerateScript(TerbitDataType td)
{
   QString res = VARIABLE_TERBIT;

   switch (td)
   {
   case TERBIT_UINT8:
      res = res.append(".UINT8");
      break;
   case TERBIT_UINT16:
      res = res.append(".UINT16");
      break;
   case TERBIT_UINT32:
      res = res.append(".UINT32");
      break;
   case TERBIT_UINT64:
      res = res.append(".UINT64");
      break;
   case TERBIT_INT8:
      res = res.append(".INT8");
      break;
   case TERBIT_INT16:
      res = res.append(".INT16");
      break;
   case TERBIT_INT32:
      res = res.append(".INT32");
      break;
   case TERBIT_INT64:
      res = res.append(".INT64");
      break;
   case TERBIT_FLOAT:
      res = res.append(".FLOAT");
      break;
   case TERBIT_DOUBLE:
      res = res.append(".DOUBLE");
      break;
   case TERBIT_SIZE_T:
      res = res.append(".SIZE_T");
      break;
   case TERBIT_UTF8:
      res = res.append(".UTF8");
      break;
   case TERBIT_BOOL:
      res = res.append(".BOOL");
      break;
   case TERBIT_DATETIME:
      res = res.append(".DATETIME");
      break;
   }

   return res;
}

QString ScriptBuilder::GenerateScript(ScriptBuilder::TypeScriptlets scriptlet, const DataClassType *type)
{
   QString res;
   switch (scriptlet)
   {
   case TYPE_FULL_NAME:
      res = GenerateFullNameScript(type);
      break;
   case TYPE_CREATE:
      res = GenerateCreateScript(type);
      break;
   case TYPE_DOCUMENTATION:
      res = GenerateDocumentationScript(type);
      break;
   }
   return res;
}


QString ScriptBuilder::GenerateScript(ScriptBuilder::InstanceScriptlets scriptlet, DataClass *dc)
{
   QString res;
   switch (scriptlet)
   {
   case INSTANCE_UNIQUE_ID:
      res = GenerateUniqueIdScript(dc);
      break;
   case INSTANCE_FIND:
      res = GenerateFindScript(dc);
      break;
   case INSTANCE_RESTORE:
      res = GenerateRestoreScript(dc);
      break;
   }
   return res;
}

QString ScriptBuilder::GenerateFullRestoreScript(Workspace *ws)
{
   ScriptBuilder script;
   BuildRestoreScript(script, ws);
   return script.ToScriptString();
}

QString ScriptBuilder::GenerateFullDocumentationScript(Workspace *ws, ScriptDocumentation* env)
{
   QString d;

   d.append(GenerateDocumentation(env));
   d.append("<br/><br/>");

   for(auto it : ws->GetDataClassManager().GetTypes())
   {
      auto t = it.second;
      d.append(GenerateDocumentationScript(t));
      d.append("<br/><br/>");
   }

   return d;
}

QString ScriptBuilder::GenerateDocumentation(Scriptlet *scriptlet)
{
   QString res;

   res.append(scriptlet->GetName());
   res.append("<br/><br/>");
   res.append(scriptlet->GetCode());
   res.append("<br/><br/>");
   res.append(scriptlet->GetDocumentation());

   return res;
}

QString ScriptBuilder::GenerateDocumentation(ScriptDocumentation *s)
{
   QString d;
   d.append(s->GetName());
   d.append("<br/><br/>");
   d.append(s->GetSummary());
   d.append("<br/><br/>");
   for(auto scriptlet : s->GetScriptlets())
   {
      d.append(GenerateDocumentation(scriptlet));
   }
   for (auto sub : s->GetSubDocumentation())
   {
      d.append("<br/><br/>");
      d.append(GenerateDocumentation(sub));
   }

   return d;
}

QString ScriptBuilder::GeneratePrintScript()
{
   return VARIABLE_TERBIT + ".Print(\"\");\n";
}

QString ScriptBuilder::GeneratePrintBrScript()
{
   return VARIABLE_TERBIT + ".PrintBr();\n";
}

QString ScriptBuilder::GenerateFullNameScript(const DataClassType *type)
{
   return ScriptEncode(type->GetFullTypeName());
}

QString ScriptBuilder::GenerateCreateScript(const DataClassType *type)
{
   QString res;
   res = QString("%1.Add(%2);").arg(VARIABLE_WORKSPACE).arg(ScriptEncode(type->GetFullTypeName()));
   return res;
}

QString ScriptBuilder::GenerateDocumentationScript(const DataClassType *type)
{
   QString res;
   res.append(type->GetFullTypeName());
   res.append("<br/><br/>");
   res = type->GetScriptDocumentation()->GetSummary();
   res.append("<br/><br/>");
   for(auto s : type->GetScriptDocumentation()->GetScriptlets())
   {
      res.append(s->GetName());
      res.append("<br/><br/>");
      res.append(s->GetCode());
      res.append("<br/><br/>");
      res.append(s->GetDocumentation());
   }
   for (auto sub : type->GetScriptDocumentation()->GetSubDocumentation())
   {
      res.append("<br/><br/>");
      res.append(GenerateDocumentation(sub));
   }

   return res;
}

QString ScriptBuilder::GenerateUniqueIdScript(DataClass *dc)
{
   return ScriptEncode(dc->GetUniqueId());
}

QString ScriptBuilder::GenerateFindScript(DataClass *dc)
{
   QString res;
   res = QString("%1.Find(%2);").arg(VARIABLE_WORKSPACE).arg(ScriptEncode(dc->GetUniqueId()));
   return res;
}

QString ScriptBuilder::GenerateRestoreScript(DataClass *dc)
{
   ScriptBuilder script;

   script.add("var dc;");
   BuildRestoreInstance(script, "dc", dc);

   return script.ToScriptString();
}


void ScriptBuilder::BuildRestoreScript(ScriptBuilder &script, Workspace* ws)
{

   //recreate data class instances
   script.add("var dc;\n");

   //recreate in proper order based on dependencies . . .
   std::list<DataClass*> dependencies;
   ws->GetDataClassManager().BuildDependencyList(dependencies);

   for(DataClass* dc : dependencies)
   {
      //only create public stuff at this level of the restore
      if (dc->GetPublicScope())
      {
         BuildRestoreInstance(script, "dc", dc);
      }
   }

   ws->BuildDockAreaRestoreScript(script, VARIABLE_WORKSPACE);
}

void ScriptBuilder::BuildRestoreInstance(ScriptBuilder &script, const QString &variableName, DataClass *dc)
{
   if (dc->IsBlock())
   {
      script.add("//////////////////////////////////////////////");
      script.add(QObject::tr("//Restore: %1\n").arg(dc->GetName()));
      BuildRestoreBlock(script, variableName, static_cast<Block*>(dc));
      script.add(QObject::tr("//End Restore: %1\n").arg(dc->GetName()));
   }
   else if (dc->IsDataSet())
   {
      //only restore remotes from this level . . . local buffers are restored/set uniqueId during block restore
      //don't restore remote indexes from here (i.e. is remote and owner is a data set)
      //remote indexes are recreated by main remote data set
      DataSet* ds = static_cast<DataSet*>(dc);
      if (ds->IsRemote())
      {
         if (!ds->GetOwner() || ds->GetOwner()->IsDataSet() == false)
         {
            script.add("//////////////////////////////////////////////");
            script.add(QObject::tr("//Restore: %1\n").arg(dc->GetName()));
            BuildRestoreRemoteDataSet(script, variableName, ds);
            script.add(QObject::tr("//End Restore: %1\n").arg(dc->GetName()));
         }
      }
   }
}

void ScriptBuilder::BuildRestoreBlock(ScriptBuilder &script, const QString &variableName, Block *dc)
{
   QString ownerUniqueId;
   if (dc->GetOwner())
   {
      ownerUniqueId = dc->GetOwner()->GetUniqueId();
   }
   //create the block
   script.add(QString("%1 = %2.Add(%3, %4, %5);").arg(variableName).arg(VARIABLE_WORKSPACE).arg(ScriptEncode(dc->GetType()->GetFullTypeName())).arg(ScriptEncode(ownerUniqueId)).arg(dc->GetPublicScope()));

   //restore settings for current block . . . this will make any outputs available as needed . . . in theory
   dc->BuildRestoreScript(script,variableName);
   //restore the name and uniqueId of the block
   BuildRestoreNameUniqueId(script,variableName,dc);

   //restore output names/uniqueId's
   QString outputVariableName = QString("%1_output").arg(variableName);
   script.add(QString("var %1;").arg(outputVariableName));

   for(auto& itr : dc->GetOutputs())
   {
      DataClass* output = itr.second;
      if (output->IsDataSet())
      {
         auto ds = static_cast<DataSet*>(output);
         script.add(QString("%1 = %2.GetOutputs().FindFirst(%3);").arg(outputVariableName).arg(variableName).arg(ds->GetOutputCategory()));
         script.add(QString("if (undefined != %1)\n{").arg(outputVariableName));
         BuildRestoreNameUniqueId(script,outputVariableName,ds);
         //also restore index buf name/info
         if (ds->GetIndexDataSet())
         {
            DataSet* idx = ds->GetIndexDataSet();

            QString idxVariableName = QString("%1_idx").arg(outputVariableName);
            script.add(QString("var %1 = %2.GetIndexDataSet();").arg(idxVariableName).arg(outputVariableName));
            script.add(QString("if (undefined != %1)\n{").arg(idxVariableName));
            BuildRestoreNameUniqueId(script,idxVariableName,idx);
            script.add("}");
         }
         script.add("}");
      }
      else
      {
         script.add(QString("%1 = %2.GetOutputs().FindFirst(%3);").arg(outputVariableName).arg(variableName).arg(output->GetOutputCategory()));
         script.add(QString("if (undefined != %1)\n{").arg(outputVariableName));
         BuildRestoreNameUniqueId(script,outputVariableName,output);
         script.add("}");
      }
   }
}

void ScriptBuilder::BuildRestoreRemoteDataSet(ScriptBuilder &script, const QString &variableName, DataSet *dc)
{
   //if remote source is based on a data set, then use the data set as the creator
   //otherwise use the remote source as the creator
   //data set can recreate index, etc. if needed . . .
   QString creatorUniqueId;
   if (dc->GetInputSource().GetOwner() && dc->GetInputSource().GetOwner()->IsDataSet())
   {
      creatorUniqueId = dc->GetInputSource().GetOwner()->GetUniqueId();
   }
   else
   {
      creatorUniqueId = dc->GetInputSource().GetUniqueId();
   }

   QString creatorVariableName = QString("%1_creator").arg(variableName);
   script.add(QString("var %1 = %2.Find(%3);").arg(creatorVariableName).arg(VARIABLE_WORKSPACE).arg(ScriptEncode(creatorUniqueId)));
   script.add(QString("if (undefined != %1)\n{").arg(creatorVariableName));

   QString ownerUniqueId;
   if (dc->GetOwner())
   {
      ownerUniqueId = dc->GetOwner()->GetUniqueId();
   }
   //create the remote data set
   script.add(QString("%1 = %2.AddRemoteDataSet(%3, %4, %5);").arg(variableName).arg(VARIABLE_WORKSPACE).arg(creatorVariableName).arg(ScriptEncode(ownerUniqueId)).arg(dc->GetPublicScope()));

   //restore the name and uniqueId of the data set
   BuildRestoreNameUniqueId(script,variableName,dc);

   //the create will also recreate the index buf . . . now set the name/uniqueId for the index
   if (dc->GetIndexDataSet())
   {
      DataSet* idx = dc->GetIndexDataSet();

      QString idxVariableName = QString("%1_idx").arg(variableName);
      script.add(QString("var %1 = %2.GetIndexDataSet();").arg(idxVariableName).arg(variableName));
      script.add(QString("if (undefined != %1)\n{").arg(idxVariableName));
      BuildRestoreNameUniqueId(script,idxVariableName,idx);
      script.add("}");
   }
   script.add("}"); //end statement block for creator found check
}

void ScriptBuilder::BuildRestoreNameUniqueId(ScriptBuilder &script, const QString &variableName, DataClass *dc)
{
   script.add(QString("%1.SetName(%2);").arg(variableName).arg(ScriptEncode(dc->GetName())));
   script.add(QString("%1.SetUniqueId(%2);").arg(variableName).arg(ScriptEncode(dc->GetUniqueId())));
}



}
