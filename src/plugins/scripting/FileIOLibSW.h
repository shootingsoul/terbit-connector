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

#include <QObject>
#include <QtQml/QJSEngine>
#include <stdint.h>
#include <QFile>
#include "tools/TerbitDefs.h"

#define QT_BEGIN_NAMESPACE
class QTextStream;
class QDataStream;
#define QT_END_NAMESPACE
namespace terbit
{

class Workspace;
class ScriptProcessor;
class ScriptDocumentation;

class FileIOLibSW : public QObject
{
   Q_OBJECT
public:
   explicit FileIOLibSW(Workspace *w, ScriptProcessor *ide, QJSEngine *engine);
   virtual ~FileIOLibSW();

   Q_INVOKABLE bool Open(const QJSValue filePathName, const QJSValue mode);
   Q_INVOKABLE void Close();

   Q_INVOKABLE bool WriteDataSet(const QJSValue &dataSet, QJSValue startIdx, QJSValue nElts);
   Q_INVOKABLE bool ReadDataSet(QJSValue &dataSet, QJSValue startIdx, QJSValue nElts);

   Q_INVOKABLE bool WriteStr(const QJSValue data);


private:
   QIODevice::OpenMode getFlags(QString modeStr);

#if 0
   Q_INVOKABLE bool WriteTxt(const QJSValue v);
   Q_INVOKABLE QJSValue ReadTxt(int type);

   Q_INVOKABLE int WriteBin(const QJSValue v, int type);
   Q_INVOKABLE QJSValue ReadBin(int type);

private:
   bool getbin(QJSValue v, char **buf, uint32_t *nBytes, int type);
   bool gettxt(QJSValue v, QString &str);

#endif
signals:

public slots:

private:
   QFile m_file;
   Workspace   *m_workspace = NULL;
   ScriptProcessor *m_ide   = NULL;
   QJSEngine       *m_se    = NULL;

   QTextStream *m_txtIn   = NULL;
   QTextStream *m_txtOut  = NULL;
   QDataStream *m_dataIn  = NULL;
   QDataStream *m_dataOut = NULL;

   QString     m_msgColor;


};


ScriptDocumentation* BuildScriptDocumentationFileIOLibSW();

} // end terbit
