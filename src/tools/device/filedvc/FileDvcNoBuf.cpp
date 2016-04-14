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
#include "FileDvcNoBuf.h"
#include <stdio.h>
#include <fcntl.h>

#if _WINDOWS
#include <io.h>
#endif

namespace terbit
{

static const QString FileDvcNoBufEmptyString;

FileDvcNoBuf::FileDvcNoBuf()
{

}

bool FileDvcNoBuf::Open(const QStringList& files, bool loop)
{
   bool retVal;
   if(m_isValid)
   {
      retVal = false; // this instance needs to be closed
   }
   else
   {
      retVal = m_isValid = ReIndex(files);
      m_loop = loop;
   }
   return retVal;
}

void FileDvcNoBuf::Close(void)
{
   for(auto it : m_fileList)
   {
      if(NULL != it.hFile)
      {
         fclose(it.hFile);
      }
   }
   m_loop         = false;
   m_eod          = false;
   m_dataPos      = 0;
   m_dataBytes    = 0;
   m_isValid      = false;
   m_fileList.clear();
}

// QString.isNull() returns true on failure
QString FileDvcNoBuf::GetCurFilePathName()
{
   if(m_fileList.size() > m_curFileIdx)
   {
      return m_fileList[m_curFileIdx].filePathName;
   }
   else
   {
      return QString();
   }
}

uint64_t FileDvcNoBuf::GetCurFileBytes()
{
   if(m_fileList.size() > m_curFileIdx)
   {
      return m_fileList[m_curFileIdx].nFileBytes;
   }
   else
   {
      return 0;
   }
}

// returns -1 if current file index is invalid
uint64_t FileDvcNoBuf::SetCurFilePos(uint64_t pos)
{
   uint64_t retVal;

   if(m_fileList.size() > m_curFileIdx)
   {

#if !_WINDOWS
      if(pos >= m_fileList[m_curFileIdx].nFileBytes)
      {
         fseeko(m_fileList[m_curFileIdx].hFile, 0, SEEK_END);
      }
      else
      {
         fseeko(m_fileList[m_curFileIdx].hFile, pos, SEEK_SET);
      }
#else
      if(pos >= m_fileList[m_curFileIdx].nFileBytes)
      {
         _fseeki64(m_fileList[m_curFileIdx].hFile, 0, SEEK_END);
      }
      else
      {
         _fseeki64(m_fileList[m_curFileIdx].hFile, pos, SEEK_SET);
      }
#endif // !_WINDOWS

      retVal = m_fileList[m_curFileIdx].curFilePos = getFilePos(m_fileList[m_curFileIdx].hFile);
      m_dataPos = calcCurDataPos();
   }
   else
   {
      retVal = -1;
   }
   return retVal;
}

// returns -1 if invalid current file index or file access failure
// TODO: should we use m_fileList[].curFilePos instead of accessing
// the file with ftell?
uint64_t FileDvcNoBuf::GetCurFilePos()
{
   uint64_t retVal = -1;
   if(m_fileList.size() > m_curFileIdx)
   {
      //retVal = getFilePos(m_fileList[m_curFileIdx].hFile);
      retVal = m_fileList[m_curFileIdx].curFilePos;
   }
   return retVal;
}


// Can only set to the last position
uint64_t FileDvcNoBuf::SetDataPos(uint64_t pos)
{
   uint64_t sum = 0;
   if(pos > m_dataBytes - 1)
   {
      pos = m_dataBytes - 1;
   }

   bool done = false;
   for(auto i = 0; i < m_fileList.size() && !done; ++i)
   {
      if(sum + m_fileList[i].nFileBytes > pos)
      {
         m_curFileIdx = i;
         SetCurFilePos(pos - sum);
         done = true;
      }
      else
      {
         sum += m_fileList[i].nFileBytes;
      }
   }

   return GetDataPos();
}


FileDvcNoBuf::~FileDvcNoBuf()
{
   Close();
}


bool FileDvcNoBuf::SeekDataBegin(void)
{
   // Set current file back to 0
   m_curFileIdx = 0;
   SeekFileBegin();

   return true;
}

void FileDvcNoBuf::SeekFileBegin()
{
    rewind(m_fileList[m_curFileIdx].hFile);
    m_fileList[m_curFileIdx].curFilePos = 0;
    m_dataPos = calcCurDataPos();
}


// Advances to beginning of the file nfiles away
// Will advance to the last file if no loop and param too large pos
// Will "advance" to the first file if no loop and param too large neg
// Parameter value of 0 will rewind current file
// If loop, then advance will be (curFile + nFiles)%numFiles in list
bool FileDvcNoBuf::AdvanceFile(int64_t nfiles)
{
   int64_t n = (int64_t)m_curFileIdx + nfiles;

   if(n >= 0)
   {
      if(m_loop)
      {
         m_curFileIdx = n % m_fileList.size();
      }
      else
      {
         m_curFileIdx += nfiles;
         if(m_curFileIdx >= m_fileList.size())
         {
            m_curFileIdx = m_fileList.size() - 1;
         }
      }
   }
   else
   {
      if(m_loop)
      {
         m_curFileIdx = (m_fileList.size() - 1) - (n % m_fileList.size() + 1);
      }
      else
      {
         m_curFileIdx = 0;
      }
   }
   SeekFileBegin();
   return true;
}


// Perhaps low performance function that closes all current files
// then re-inits with the passed list.
bool FileDvcNoBuf::ReIndex(const QStringList &filePaths)
{
   bool retVal = false;

   Close();

   for(auto it : filePaths)
   {
      FileInfo_t fi;
      fi.curFilePos = 0;
      fi.nFileBytes = 0;

#pragma warning( disable : 4996 )
      fi.hFile = fopen(it.toStdString().c_str(), "rb");
#pragma warning( default : 4996 )

      if(NULL != fi.hFile)
      {
         retVal = true;
         fi.nFileBytes = getFileBytes(fi.hFile);
         m_dataBytes += fi.nFileBytes;
         fi.filePathName = it;
         fi.curFilePos = 0;
         m_fileList.push_back(fi);
      }
   }

   m_eod = (m_dataBytes == 0);

   m_curFileIdx = 0;
   m_dataPos    = 0;

   return retVal;
}

uint64_t FileDvcNoBuf::calcCurDataPos()
{
   uint64_t retVal = 0;
   for(auto i = 0; i < m_curFileIdx; ++i)
   {
      retVal += m_fileList[i].nFileBytes;
   }

   return retVal += m_fileList[m_curFileIdx].curFilePos;
}

// returns -1 if file access failure
uint64_t FileDvcNoBuf::getFilePos(FILE *hFile)
{
   uint64_t retVal;
#if !_WINDOWS
         retVal = ftello(hFile);
#else
         retVal = _ftelli64(hFile);
#endif // !_WINDOWS

  return retVal;
}

// Note: this function resets pointer to beginning of file!
uint64_t FileDvcNoBuf::getFileBytes(FILE* hFile)
{
   uint64_t retVal = 0;

#if !_WINDOWS
         fseeko(hFile, 0, SEEK_END);
         retVal = ftello(hFile);
#else
         _fseeki64(hFile, 0, SEEK_END);
         retVal = _ftelli64(hFile);
#endif // !_WINDOWS

   rewind(hFile);
   return retVal;
}

// This helper function assumes that it has already been determined that we
// have a valid file and buffer and that we need to read data from the file
// into the buffer.
// This function will fill the buffer starting at offset 0 until
// it is full or we run out of file data.
// Therefore, is is assumed that the file is too big to fit into
// a single buffer.
// This function will set m_eof.
uint64_t FileDvcNoBuf::Read(void* dst, size_t eltSize, uint64_t nElts)
{
   uint64_t bytesXfrd = 0;
   uint64_t bytesRead;
   bool done = false;
   uint64_t bytes2Xfr = nElts * eltSize;

   // while we haven't filled the buffer,
   // Read from current file.
   // Open next file if necessary
   // Mark EOD if we have reached end of data by our accounting
     // because eof not indicated from file system until you try to read past end.
   while(bytesXfrd < bytes2Xfr && !done && !m_eod)
   {
      if(m_curFileIdx < m_fileList.size())// safety check
      {
         bytesRead = fread((char*)dst + bytesXfrd, 1, bytes2Xfr - bytesXfrd, m_fileList[m_curFileIdx].hFile);
         m_fileList[m_curFileIdx].curFilePos += bytesRead;
         if(bytesRead < bytes2Xfr - bytesXfrd)
         {
            // Check for EOF.
            if(feof(m_fileList[m_curFileIdx].hFile))
            {
               bool rollover;
               done = !getNextFile(rollover);
               if(!rollover)
               {
                  m_dataPos += bytesRead;
               }
            }
            else
            {
               // some sort of error
               // TODO: signal access failure
               clearerr(m_fileList[m_curFileIdx].hFile);
               done = true;
            }
         }
         else
         {
            m_dataPos += bytesRead;
         }
         bytesXfrd += bytesRead;
      }
      else
      {
         // TODO: assert error signal
         done = true;
      }

      m_eod = (m_dataPos == m_dataBytes) && !m_loop;
   }

   return bytesXfrd;
}

bool FileDvcNoBuf::getNextFile(bool& rollover)
{
   bool retVal = false;
   rollover = false;
   if(m_curFileIdx + 1 == m_fileList.size() && !m_loop)
   {
      // if we're already on the last file and not looping, then done
      ;
   }
   else
   {
      if(m_curFileIdx + 1 < m_fileList.size())
      {
         m_curFileIdx++;
         retVal = true;
      }
      else if(m_loop)
      {
         rollover = true;
         m_curFileIdx = 0;
         m_dataPos = 0;
         retVal = true;
      }

      if(retVal)
      {
         rewind(m_fileList[m_curFileIdx].hFile);
         m_fileList[m_curFileIdx].curFilePos = 0;
      }
   }
   return retVal;
}

uint64_t FileDvcNoBuf::Write(const void* src, size_t eltSize, uint64_t nElts)
{
   size_t retVal = 0;
   // not implemented for this device.
   src; eltSize; nElts;
   return retVal;
}

}// namespace terbit
