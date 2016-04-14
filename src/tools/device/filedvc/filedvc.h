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

#ifndef FILEDVC_H
#define FILEDVC_H

#include <stdint.h>
#include <QString>
#include <stdio.h>

namespace terbit
{
/*** The FileDvc class improves performance for the FileDvcIDL by
 * "cache-ing" file data in RAM.  It can also provide a never-ending
 * source of data by seamlessly handling buffer and file boundaries.
 * If feasible, the entire file is stored in RAM; data is then pulled from
 * RAM in a circular fashion.  If the entire file cannot be stored in RAM,
 * then the files and buffers must be carefully managed.
 * There are various ways to approach this - one circular buffer (like a
 * FIFO), ping-pong buffers, multiple buffers, etc.
 **************************************************************/
class FileDvc
{
public:
   FileDvc();
   ~FileDvc();

   // Basic device functions
   uint64_t Read(void* dst, size_t eltSize, uint64_t nElts, const void* src);
   uint64_t Write(const void* src, size_t eltSize, uint64_t nElts, void* dst);
   bool   Open(const QString& filename, bool loop = false);
   void   Close(void);

   // Accessor Functions
   void SetLoop(bool loop){m_loop = loop;}
   void SetFilePathName(const QString& p){m_filePathName = p;}

   bool     GetLoop(void){return m_loop;}
   const    QString&  GetFilePathName(void){return m_filePathName;}
   size_t   GetBufBytes(void){return m_bufBytes;}
   uint32_t GetNumBufs(void){return m_nBufs;}
   uint64_t GetFileBytes(void){return  m_fileBytes;}
   bool     GetEof(void){return m_eod;}
   bool     IsValid(){return m_isValid;}
   uint64_t GetFilePos(void){return m_filePos;}
   uint64_t SeekBegin(void);


private:
   bool initialize(const QString& filename, bool loop, bool newFile);
   size_t fillDataBuffer(char* dst);

   bool      m_loop;
   bool      m_eof; // end of file
   bool      m_eod; // end of data - eof + all data read from buffer
   bool      m_isValid;
   QString   m_filePathName;
   uint32_t  m_nBufs;
   size_t    m_bufBytes;
   FILE*     m_pFile;
   size_t    m_readPtr;
   uint64_t    m_filePos;
   size_t    m_endDataPtr;
   uint64_t    m_fileBytes;
   char*     m_pBuf;
};
}// namespace terbit
#endif // FILEDVC_H
