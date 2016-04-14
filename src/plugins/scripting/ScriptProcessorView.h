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

#include <QAction>
#include <QMenu>
#include "ScriptBuilder.h"
#include <connector-core/WorkspaceDockWidget.h>

QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QAction)

namespace terbit
{

class CodeEditor;
class ScriptProcessor;
class Scriptlet;
class ScriptDocumentation;

class ScriptProcessorView : public WorkspaceDockWidget
{
   Q_OBJECT
public:
   ScriptProcessorView(ScriptProcessor* ide);
   ~ScriptProcessorView();

   QString GetFullSourceCode();

private slots:
   void OnExecute();
   void OnFileOpen();
   void OnSave();
   void OnSaveAs();

   void OnMenuShowInstances();
   void OnMenuShowInstanceScriptlets();
   void OnMenuInstanceScriptlet();

   void OnMenuShowTypes();
   void OnMenuShowTypeScriptlets();
   void OnMenuTypeScriptlet();
   void OnMenuCustomScriptlet();

   void OnMenuShowEnvScriptlets();
   void OnMenuScriptDoc();

   void OnFullRestoreScriptlet();
   void OnFullDocumentationScriptlet();

   void OnPrintScriptlet();
   void OnPrintBrScriptlet();
   void OnScriptletSense();
   void OnShutdownEngine();
   void OnFontSizeChange(int);

   void OnUndoAvailable(bool available);

   void OnModelNameChanged(DataClass*);
   void OnModelSourceCodeLoaded();
   void OnModelPrint(const QString& html, const QString& colorName);
   void OnModelPrintBr(const QString& html, const QString& colorName);
   void OnModelExecutionStart();
   void OnModelExecutionEnd();
   void OnModelScriptSaved();

private:
   bool SaveScript();
   bool SaveScriptAs();
   void SetupKeyboardShortcuts();
   void ApplyScriptlet(const QString& script, bool intellisense);
   void ApplyFontSize();

   CodeEditor* m_editor;
   ScriptProcessor* m_ide;
   QTextEdit* m_messages;
   QComboBox* m_fontSize;
   bool m_loading;
   QAction* m_save;
   QToolButton* m_execute;
   QToolButton* m_shutdown;
   ScriptDocumentation* m_environmentDocumentation;

   //menu helper classes and functions
   class TypeScriptletMenu : public QMenu
   {
   public:
      TypeScriptletMenu(QWidget* parent, const QIcon& icon, const QString& text, DataClassType* type, bool forIntellisense);
      DataClassType* dataClassType;
      bool intellisense;
   };


   class CustomScriptletAction : public QAction
   {
   public:
      CustomScriptletAction(QObject* parent, Scriptlet* s, bool forIntellisense);
      Scriptlet* scriptlet;
      bool intellisense;
   };

   class ScriptDocumentationAction : public QAction
   {
   public:
      ScriptDocumentationAction(QObject* parent, ScriptDocumentation* d, bool forIntellisense);
      ScriptDocumentation* scriptDocumentation;
      bool intellisense;
   };

   class TypeScriptletAction : public QAction
   {
   public:
      TypeScriptletAction(QObject* parent, const DataClassType* type, ScriptBuilder::TypeScriptlets s, bool forIntellisense);
      const DataClassType* dataClassType;
      ScriptBuilder::TypeScriptlets scriptlet;
      bool intellisense;
   };

   class EnvScriptletMenu : public QMenu
   {
   public:
      EnvScriptletMenu(QWidget* parent, ScriptDocumentation* d, bool forIntellisense);
      ScriptDocumentation* scriptDocumentation;
      bool intellisense;
   };

   class InstanceScriptletMenu : public QMenu
   {
   public:
      InstanceScriptletMenu(QWidget* parent, const QIcon& icon, const QString& text, DataClass* dc, bool forIntellisense);
      DataClass* dataClass;
      bool intellisense;
   };

   class InstanceScriptletAction : public QAction
   {
   public:
      InstanceScriptletAction(QObject* parent, DataClass* dc, ScriptBuilder::InstanceScriptlets s, bool forIntellisense);
      DataClass* dataClass;
      ScriptBuilder::InstanceScriptlets scriptlet;
      bool intellisense;
   };

   void BuildScriptletMenu(QMenu* menu, bool intellisense);
   void AddInstancesToScriptletMenu(InstanceScriptletMenu* menu);
   void AddTypesToScriptletMenu(TypeScriptletMenu *menu);
};


}
