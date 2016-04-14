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
#include <QString>
#include <QStringList>
#include <stdio.h>

namespace terbit
{
/*** This version of the FileDvc class abstracts the access of multiple
 * files so that it appears as a single large data source.  The performance
 * is de-emphasized in favor of a "safer" open-read-close process.  This
 * allows us to better handle cases where the file or files are modified
 * or removed by other agents in the system.
 * This class can be configured to run in a loop, effectively providing
 * an unending source of data.
 * This class will provide read, write, and seek functions, as well
 * as informational data such as what the current file is.
 * This class could provide advanced functions such as advancing or
 * reverting to a previous file (instead of by a certain number of bytes).
 * This class will provide a "re-index" function that will cause it
 * to close all files, re-evaluate the passed list of files, and
 * recalculate the size.
 * This class will emit several signals on certain events such
 * as crossing into a new file or having a file access failure.
 **************************************************************/

class FileDvcNoBuf
{
public:
   FileDvcNoBuf();
   ~FileDvcNoBuf();

   // Basic device functions
   uint64_t Read(void* dst, size_t eltSize, uint64_t nElts);
   uint64_t Write(const void* src, size_t eltSize, uint64_t nElts);
   bool   Open(const QStringList& files, bool loop = false);
   void   Close(void);

   // Accessor Functions
   void     SetLoop(bool loop){m_loop = loop;}
   bool     GetLoop(void){return m_loop;}
   QString  GetCurFilePathName(void);
   uint64_t GetCurFileBytes(void);
   uint64_t     SetCurFilePos(uint64_t pos);
   uint64_t GetCurFilePos(void);
   bool     GetEod(void){return m_eod;}
   bool     IsValid(){return m_isValid;}
   uint64_t GetDataPos(void){return m_dataPos;}
   uint64_t SetDataPos(uint64_t pos);
   uint64_t GetDataBytes(void){return m_dataBytes;}
   bool     SeekDataBegin(void);
   void     SeekFileBegin(void);
   bool     AdvanceFile(int64_t nfiles);
   bool     ReIndex(const QStringList& filePaths);
   // TODO: a reindex that takes a file that could be more surgical


   // ---------------- Test functions ----------------
   // DO NOT USE these in application code
   int TEST_GetCurFileIdx(){return m_curFileIdx;}


//signals:
   // Access error
   // File transition
   // read complete?  index complete? close complete?
private:
   uint64_t getFileBytes(FILE *hFile);
   bool getNextFile(bool &rollover);
   uint64_t calcCurDataPos();
   uint64_t getFilePos(FILE* hFile);

private:
   typedef struct
   {
      FILE* hFile;
      uint64_t nFileBytes;
      QString filePathName;
      uint64_t curFilePos;
   }FileInfo_t;

   bool initialize(const QStringList& filename, bool loop);

   bool       m_loop = false;
   bool       m_eod = false; // end of data - eof + all data read from buffer
   bool       m_isValid = false;
   FileInfo_t m_curFileInfo;
   size_t     m_curFileIdx = 0;
   uint64_t   m_dataPos    = 0;
   uint64_t   m_dataBytes  = 0;
   std::vector<FileDvcNoBuf::FileInfo_t> m_fileList;
};
}// namespace terbit
