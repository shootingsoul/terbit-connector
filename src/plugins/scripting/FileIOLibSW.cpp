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
#include "FileIOLibSW.h"
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <tools/Tools.h>
#include "ScriptProcessor.h"
#include "connector-core/Workspace.h"
#include "connector-core/ScriptDocumentation.h"

namespace terbit
{


FileIOLibSW::FileIOLibSW(Workspace *w, ScriptProcessor *ide, QJSEngine *engine) : m_workspace(w), m_ide(ide), m_se(engine)
{
   m_msgColor = "red";
}

FileIOLibSW::~FileIOLibSW()
{
   Close();
   delete m_txtIn;
   delete m_txtOut;
   delete m_dataIn;
   delete m_dataOut;
}

// mode is pseudo cstdio mode string
// "r" "w" "a" "rb" "wb" "ab" "r+" "r+b", "rb+", etc...
// only binary allowed for now
bool FileIOLibSW::Open(const QJSValue filePathName, const QJSValue mode)
{
   bool retVal = false;


   if(mode.isString() && filePathName.isString())
   {

      m_file.setFileName(filePathName.toString());

      // Setup stream based on mode
      QIODevice::OpenMode flags = getFlags(mode.toString());

      if(m_file.open(flags))
      {
         if(flags & QIODevice::Text)
         {
            if(flags & QIODevice::ReadOnly)
            {
               m_txtIn = new QTextStream(&m_file);
            }
            if(flags & QIODevice::WriteOnly)
            {
               m_txtOut = new QTextStream(&m_file);
            }
         }
         else
         {
            if(flags & QIODevice::ReadOnly)
            {
               m_dataIn = new QDataStream(&m_file);
            }
            if(flags & QIODevice::WriteOnly)
            {
               m_dataOut = new QDataStream(&m_file);
            }
         }
         retVal = true;
      }
      else
      {
         emit m_ide->Print(tr("File Open failed."), m_msgColor);
      }
   }
   else
   {
      emit m_ide->Print(tr("File open parameter error.  Both parameters must be strings."), m_msgColor);
   }

   if(!retVal)
   {

      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("File open failed."));
   }

   return retVal;
}

void FileIOLibSW::Close()
{
   m_file.close();
}

bool FileIOLibSW::WriteDataSet(const QJSValue &dataSet, QJSValue startIdx, QJSValue nElts)
{
   bool retVal = false;
   uint64_t start = 0;
   size_t numElts = 0;

   if(m_dataOut)
   {
      DataClass* dc = m_workspace->FindInstance(dataSet);
      if (dc && dc->IsDataSet())
      {
         auto ds = static_cast<DataSet*>(dc);
         if(startIdx.isNumber())
         {
            start = (uint64_t)startIdx.toNumber();
            if(start >= ds->GetCount())
            {
               start = ds->GetCount() - 1;
            }
         }

         if(nElts.isNumber())
         {
            numElts = (size_t)nElts.toNumber();
            if(start + numElts > ds->GetCount())
            {
               numElts = ds->GetCount() - (start + 1);
            }
         }
         else
         {
            numElts = ds->GetCount() - (start + 1);
         }

         retVal = true;

         for(size_t i = 0; i < numElts && retVal; ++i)
         {
            char buf[8];
            uint32_t nBytes = TerbitDataTypeSize(ds->GetDataType());

            if(TERBIT_INT8 == ds->GetDataType())
            {
               int8_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_UINT8 == ds->GetDataType())
            {
               uint8_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_INT16 == ds->GetDataType())
            {
               int16_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_UINT16 == ds->GetDataType())
            {
               uint16_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_INT16 == ds->GetDataType())
            {
               int16_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_UINT32 == ds->GetDataType())
            {
               uint32_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_INT64 == ds->GetDataType())
            {
               int64_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_UINT64 == ds->GetDataType())
            {
               uint64_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_FLOAT == ds->GetDataType())
            {
               float val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_SIZE_T == ds->GetDataType())
            {
               size_t val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else if(TERBIT_DOUBLE == ds->GetDataType())
            {
               double val = ds->GetValueAtIndex(i + start);
               memset(buf, 0, 8);
               memcpy(buf, &val, nBytes);
            }
            else
            {
               emit m_ide->Print(tr("Invalid dataset data type for file write.").arg(i + start), m_msgColor);
               retVal = false;
            }


            if (retVal && nBytes != m_dataOut->writeRawData(buf, nBytes))
            {
               emit m_ide->Print(tr("Failed to write a full double value at index %1.").arg(i + start), m_msgColor);
               retVal = false;
            }
         }
      }
      else
      {
         emit m_ide->Print(tr("Write DataSet invalid input data set."), m_msgColor);
      }
   }
   else
   {
      emit m_ide->Print(tr("File is not configured for binary mode writing."), m_msgColor);
   }

   if(!retVal)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("DataSet file write failed."));
   }
   return retVal;
}

bool FileIOLibSW::ReadDataSet(QJSValue &dataSet, QJSValue startIdx, QJSValue nElts)
{

   bool retVal = false;
   uint64_t start = 0;
   size_t numElts = 0;

   if(m_dataIn)
   {
      DataClass* dc = m_workspace->FindInstance(dataSet);
      if (dc && dc->IsDataSet())
      {
         auto ds = static_cast<DataSet*>(dc);
         if(startIdx.isNumber())
         {
            start = (uint64_t)startIdx.toNumber();
            if(start >= ds->GetCount())
            {
               start = ds->GetCount() - 1;
            }
         }

         if(nElts.isNumber())
         {
            numElts = (size_t)nElts.toNumber();
            if(start + numElts > ds->GetCount())
            {
               numElts = ds->GetCount() - (start + 1);
            }
         }
         else
         {
            numElts = ds->GetCount() - (start + 1);
         }


         retVal = true;

         for(size_t i = 0; i < numElts && retVal; ++i)
         {
            char buf[8];
            memset(buf, 0, 8);
            size_t nBytes = TerbitDataTypeSize(ds->GetDataType());
            if(8 < nBytes)
            {
               emit m_ide->Print(tr("Invalid dataset data type for file read.").arg(i + start), m_msgColor);
               retVal = false;
            }

            if(!m_dataIn->atEnd())
            {
               if(nBytes == m_dataIn->readRawData(buf, nBytes))
               {
                  ds->SetValueAtIndex(i + start, *(double*)buf);
               }
               else
               {
                  emit m_ide->Print(tr("Failed to read a full value at index %1.").arg(i + start), m_msgColor);
                  retVal = false;
               }
            }
            emit m_ide->Print(tr("Failed to read a full range due to end of file.").arg(i + start), m_msgColor);
            retVal = false;
         }
      }
      else
      {
         emit m_ide->Print(tr("Read DataSet invalid input data set."), m_msgColor);
      }
   }
   else
   {
      emit m_ide->Print(tr("File is not configured for binary mode reading."), m_msgColor);
   }

   if(!retVal)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("DataSet file read failed."));
   }
   return retVal;
}


bool FileIOLibSW::WriteStr(const QJSValue data)
{
   bool retVal = false;

   if(m_txtOut)
   {
      if(data.isString())
      {
         *m_txtOut << data.toString().toUtf8();
         retVal = true;
      }
      else
      {
         emit m_ide->Print(tr("Invalid type passed to WriteTxt(). Only strings are allowed."), m_msgColor);
      }
   }
   else
   {
      emit m_ide->Print(tr("File is not configured for text mode writing."), m_msgColor);
   }

   if(!retVal)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("Text file write failed."));
   }
   return retVal;
}



#if 0
QJSValue FileIOLibSW::ReadTxt(int type)
{
   QJSValue retVal;
   bool success = false;
   double number;
   QString str;

   if(m_txtIn)
   {
      if(m_txtIn->atEnd())
      {
         emit m_ide->Print(tr("End of file, unable to read"), m_msgColor);
      }
      else
      {
         QDateTime *dt;
         switch(static_cast<TerbitDataType>(type))
         {

#if 0
         case TERBIT_DATETIME:
            dt = new QDateTime;
            if(dt->timeSpec() == Qt::UTC)
            {
               dt->fromString(m_txtIn->read(26), Qt::ISODate);
            }
            else
            {
               dt->fromString(m_txtIn->read(19), Qt::ISODate);
            }
            retVal = m_se->newObject(dt);
            success = true;
            break;
         case TERBIT_DOUBLE:
            number = QString::toDouble(m_txtIn);
            retVal = number;
            success = true;
            break;
#endif
         case TERBIT_UTF8:
            retVal = m_txtIn->readLine();
            success = true;
            break;

         default:
            emit m_ide->Print(tr("Invalid type parameter passed to ReadTxt()."), m_msgColor);
            emit m_ide->Print("This function only accepts the following Terbit types:", m_msgColor);
                  //emit m_ide->Print("TERBIT_INT8");
                  //emit m_ide->Print("TERBIT_UINT8");
                  //emit m_ide->Print("TERBIT_INT16");
                  //emit m_ide->Print("TERBIT_UINT16");
                  //emit m_ide->Print("TERBIT_INT32");
                  //emit m_ide->Print("TERBIT_UINT32");
                  //emit m_ide->Print("TERBIT_INT64");
                  //emit m_ide->Print("TERBIT_UINT64");
                  //emit m_ide->Print("TERBIT_FLOAT");
           //emit m_ide->Print("TERBIT_DOUBLE", m_msgColor);
           //emit m_ide->Print("TERBIT_DATETIME", m_msgColor);
           emit m_ide->Print("TERBIT_UTF8", m_msgColor);
            break;

         }
      }
   }
   else
   {
      emit m_ide->Print( tr("File is not configured for text mode reading."), m_msgColor);
   }

   if(!success)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("Text file read failed."));
   }

   return retVal;
}


int FileIOLibSW::WriteBin(const QJSValue v, int type)
{
   bool retVal = false;
   char* buf;
   uint32_t nBytes;

   if(m_dataOut)
   {
      QString str;
      if(getbin(v, &buf, &nBytes, type))
      {
         if(nBytes == m_dataOut->writeRawData(buf, nBytes))
         {
            retVal = true;
         }
         else
         {
            emit m_ide->Print(tr("Failed to write all bytes to file."), m_msgColor);
         }
      }
   }
   else
   {
      emit m_ide->Print(tr("File is not configured for binary mode writing."), m_msgColor);
   }

   if(!retVal)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("Binary file write failed."));
   }
   return retVal;
}

QJSValue FileIOLibSW::ReadBin(int type)
{
   QJSValue retVal = false;
   bool success = false;
   char* buf;
   uint32_t nBytes;

   if(m_dataIn)
   {

      if(m_dataIn->atEnd())
      {
         emit m_ide->Print(tr("End of file, unable to read"), m_msgColor);
      }
      else
      {
         int8_t i8;
         switch(static_cast<TerbitDataType>(type))
         {
#if 0
         case TERBIT_DATETIME:
            QDateTime dt = new QDateTime;
            m_dataIn >> dt;
            retVal = m_se->newObject(new QDateTime);
            success = true;
            break;
#endif
         case TERBIT_INT8:
            m_dataIn->readRawData((char*)&i8, sizeof(i8));
            retVal = i8;
            success = true;
            break;
            break;

         default:
            emit m_ide->Print(tr("Invalid type parameter passed to ReadBin()."), m_msgColor);
            emit m_ide->Print("This function only accepts the following Terbit types:", m_msgColor);
            emit m_ide->Print("TERBIT_INT8", m_msgColor);
            //emit m_ide->Print("TERBIT_UINT8");
            //emit m_ide->Print("TERBIT_INT16");
            //emit m_ide->Print("TERBIT_UINT16");
            //emit m_ide->Print("TERBIT_INT32");
            //emit m_ide->Print("TERBIT_UINT32");
            //emit m_ide->Print("TERBIT_INT64");
            //emit m_ide->Print("TERBIT_UINT64");
            //emit m_ide->Print("TERBIT_FLOAT");
            //emit m_ide->Print("TERBIT_UTF8");
            //emit m_ide->Print("TERBIT_SIZE_T");
            break;

         }
      }
   }
   else
   {
      emit m_ide->Print(tr("File is not configured for binary mode reading."), m_msgColor);
   }

   if(!success)
   {
      LogError2(m_ide->GetType()->GetLogCategory(),m_ide->GetName(), tr("Binary file read failed."));
   }
   return retVal;

}


 bool FileIOLibSW::getbin(QJSValue v, char** buf, uint32_t* nBytes, int type)
 {
    bool retVal = true;
    uint64_t tmp;

    if(v.isNumber())
    {
       *nBytes = TerbitDataTypeSize(static_cast<TerbitDataType>(type));
       *buf = new char[*nBytes];
       switch(type)
       {
       case TERBIT_INT8:
          tmp = (int8_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_UINT8:
          tmp = (uint8_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_INT16:
          tmp = (int16_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_UINT16:
          tmp = (uint16_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_INT32:
          tmp = (int32_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_UINT32:
          tmp = (uint32_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_INT64:
          tmp = (int64_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_UINT64:
          tmp = (uint64_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_FLOAT:
          tmp = (float)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_SIZE_T:
          tmp = (size_t)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       case TERBIT_DOUBLE:
          tmp = (double)v.toNumber();
          memcpy(*buf, &tmp, *nBytes);
          break;
       default:
          retVal = false;
          break;
       }
    }
    else
    {
       retVal = false;
    }

    if(!retVal)
    {
       // output message to IDE
       emit m_ide->Print("Invalid parameter passed to WriteBin().  This function", m_msgColor);
       emit m_ide->Print("Only accepts a Number and the following Terbit types:", m_msgColor);
       emit m_ide->Print("TERBIT_INT8", m_msgColor);
       emit m_ide->Print("TERBIT_UINT8", m_msgColor);
       emit m_ide->Print("TERBIT_INT16", m_msgColor);
       emit m_ide->Print("TERBIT_UINT16", m_msgColor);
       emit m_ide->Print("TERBIT_INT32", m_msgColor);
       emit m_ide->Print("TERBIT_UINT32", m_msgColor);
       emit m_ide->Print("TERBIT_INT64", m_msgColor);
       emit m_ide->Print("TERBIT_UINT64", m_msgColor);
       emit m_ide->Print("TERBIT_FLOAT", m_msgColor);
       emit m_ide->Print("TERBIT_DOUBLE", m_msgColor);
       emit m_ide->Print("TERBIT_SIZE_T", m_msgColor);
    }

    return retVal;
 }


 bool FileIOLibSW::gettxt(QJSValue v, QString& str)
 {
    bool retVal = false;

    if (v.isDate())
    {
       str = v.toDateTime().toString(Qt::ISODate);
       retVal = true;
    }
    else if(v.isNumber())
    {
       str = DoubleToStringComplete(v.toNumber());
       retVal = true;
    }
//    else if(v.isObject())
//    {
//
//    }
//    else if(v.isQObject())
//    {
//
//    }
//    else if(v.isVariant())
//    {
//
//    }
    else if(v.isString())
    {
       str = v.toString();
       retVal = true;
    }

    if(!retVal)
    {
       // output message to IDE
       emit m_ide->Print("Invalid parameter passed to WriteTxt().  This function", m_msgColor);
       emit m_ide->Print("Only accepts a following types:", m_msgColor);
       emit m_ide->Print("DateTime", m_msgColor);
       emit m_ide->Print("Number", m_msgColor);
       emit m_ide->Print("String", m_msgColor);
    }

    return retVal;
 }

#endif

 QIODevice::OpenMode FileIOLibSW::getFlags(QString modeStr)
 {
    QIODevice::OpenMode retVal = QIODevice::Unbuffered;

    if(modeStr.contains('r'))
    {
       if(modeStr.contains('+'))
       {
          retVal |= QIODevice::ReadWrite;
       }
       else
       {
          retVal |= QIODevice::ReadOnly;
       }
    }
    else if(modeStr.contains('w'))
    {
       if(modeStr.contains('+'))
       {
          retVal |= QIODevice::ReadWrite | QIODevice::Truncate;
       }
       else
       {
          retVal |= QIODevice::WriteOnly | QIODevice::Truncate;
       }
    }
    else if(modeStr.contains('a'))
    {
       if(modeStr.contains('+'))
       {
          retVal |= QIODevice::ReadWrite | QIODevice::Append;
       }
       else
       {
          retVal |= QIODevice::WriteOnly | QIODevice::Append;
       }
    }

    if(modeStr.contains('b'))
    {
       ; // default mode
    }
    else
    {
       // only binary for now
       //emit m_ide->Print(tr("Text mode not valid.  Use binary mode."), m_msgColor);
       //retVal = false;
       retVal |= QIODevice::Text;
    }


    return retVal;
 }


 ScriptDocumentation* BuildScriptDocumentationFileIOLibSW()
 {
    ScriptDocumentation* d = new ScriptDocumentation();

    d->SetName(FileIOLibSW::tr("File IO Lib"));
    d->SetSummary(FileIOLibSW::tr("A FileIOLibSW provides a wrapper to a subset of QFile functionality, allowing binary reads and writes to the file system."));

    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("CreateFileIO"),"terbit.CreateFileIO();",FileIOLibSW::tr("Create an instance of a FileIOLibSW object, which allows reading and writing of files.")));
    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("Open"), "Open(filePathName, mode);", FileIOLibSW::tr("Opens the file specified by the filePathName string using the modes defined by the mode string.")));
    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("Close"), "Close();",FileIOLibSW::tr("Closes the file.")));
    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("WriteDataSet"), "WriteDataSet(dataSet, startIdx, nElts);",FileIOLibSW::tr("Writes nElts of data from the data set, starting at startIdx (of the dataset) to the file.")));
    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("ReadDataSet"), "ReadDataSet(dataSet, startIdx, nElts);",FileIOLibSW::tr("Reads nElts elements of data from the file, into the dataset, starting at startIdx.")));
    d->AddScriptlet(new Scriptlet(FileIOLibSW::tr("WriteStr"), "WriteStr(str);",FileIOLibSW::tr("Writes the string to the file, similar to writeln.")));

    return d;
 }

} // end terbit

