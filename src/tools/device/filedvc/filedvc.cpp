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

#include "filedvc.h"
#include <stdio.h>
#include <fcntl.h>

#if _WINDOWS
#include <io.h>
#endif

namespace terbit
{

static const uint32_t g_MaxBufBytes = 0x02000000;
static const uint32_t g_MinBufBytes = 0x00100000;

// we probably won't need more than 2 buffers until we do asynch file i/o
static const uint32_t g_MaxNumBufs  = 4;

FileDvc::FileDvc()
{
   m_loop         = false;
   m_nBufs        = 0;
   m_bufBytes     = 0;
   m_pFile        = NULL;
   m_pBuf         = NULL;
   m_eof          = false;
   m_eod          = false;
   m_filePos      = 0;
   m_isValid      = false;
}

bool FileDvc::Open(const QString& filename, bool loop)
{
   bool retVal;
   if(m_isValid)
   {
      retVal = false; // this instance needs to be closed
   }
   else
   {
      retVal = m_isValid = initialize(filename, loop, true);

   }
   return retVal;
}

void FileDvc::Close(void)
{
   if(m_pFile)
   {
      fclose(m_pFile);
      m_pFile = NULL;
   }
   m_loop         = false;
   m_eof          = false;
   m_eod          = false;
   m_filePos      = 0;
   m_isValid      = false;
}


FileDvc::~FileDvc()
{
   if(NULL != m_pFile)
   {
      fclose(m_pFile);
   }
   delete [] m_pBuf;
}

bool FileDvc::initialize(const QString& filename, bool loop, bool newFile)
{
   bool retVal = false;
   if(newFile)
   {
      m_loop = loop;
      m_nBufs        = 0;
      m_bufBytes     = 0;
      m_filePathName = filename;
   }


   // Try to open new file, get RAM buffer.
   // future optimizations - multiple smaller buffers when a
   // single large buffer is not enough
   if(newFile)
   {
#pragma warning( disable : 4996 )
#if 1
      m_pFile = fopen(filename.toStdString().c_str(), "rb");
#else
      m_pFile = (FILE*)_open(filename.toStdString().c_str(), _O_RDONLY | _O_BINARY);
      m_pFile = (-1 == (int)m_pFile)?NULL:m_pFile; // convert -1 to NULL
#endif //!_WINDOWS
#pragma warning( default : 4996 )
   }
   if(NULL == m_pFile)
   {
      ; // return failure
   }
   else // Get RAM buffer
   {
      char* pBuf = NULL;
      uint64_t bufSize = m_fileBytes;

      if(newFile)
      {
#if !_WINDOWS
         fseeko(m_pFile, 0, SEEK_END);
         bufSize = m_fileBytes = ftello(m_pFile);
#else
         _fseeki64(m_pFile, 0, SEEK_END);
         bufSize = m_fileBytes = _ftelli64(m_pFile);
#endif // !_WINDOWS
      }
      rewind(m_pFile);
      m_filePos = 0;
#if !_WINDOWS
      m_eof = (m_fileBytes == ftello(m_pFile));
#else
      m_eof = (m_fileBytes == _ftelli64(m_pFile));
#endif

      if(bufSize <= m_bufBytes)
      {
         // we already have a buffer that will hold the data
      }
      else
      {
         delete []m_pBuf;
         m_pBuf = NULL;

         // Now create new buffers
         if(bufSize > g_MaxBufBytes)
         {
            bufSize = g_MaxBufBytes;
         }

         // Get a big buffer, if we can't, try a smaller
         pBuf = new(std::nothrow) char[bufSize];
         while(NULL == pBuf && bufSize >= g_MinBufBytes)
         {
            bufSize /= 2;
            pBuf = new(std::nothrow) char[bufSize];
         }

         if(NULL != pBuf)
         {
            m_bufBytes = bufSize;
            m_nBufs  = 1;
         }

         m_pBuf = pBuf;
      }

      if( m_fileBytes == 0)
      {
         m_bufBytes    = 0;
         m_nBufs       = 0;
         m_readPtr     = 0;
         m_filePos     = 0;
         m_endDataPtr  = 0;
         m_eof = m_eod = true;
         retVal        = true;
      }
      else if(NULL == m_pBuf)
      {
         m_bufBytes    = 0;
         m_nBufs       = 0;
         m_readPtr     = 0;
         m_filePos     = 0;
         m_endDataPtr  = 0;
         m_eof = m_eod = true;
      }
      else
      {         
         retVal = true;
         // Fill buffer with data from file.
#if 1
         size_t temp = fread(m_pBuf, 1, bufSize, m_pFile);
#else
         size_t temp = _read((int)m_pFile, m_pBuf, bufSize);
#endif //!_WINDOWS
         if(temp != bufSize && temp != m_fileBytes)
         {
            // we should have read a full buffer or the whole file
            retVal = false;
         }
#if !_WINDOWS
         m_eof = (m_fileBytes ==  ftello(m_pFile));
#else
         m_eof = (m_fileBytes == _ftelli64(m_pFile));
#endif // !_WINDOWS
         m_endDataPtr = temp;
         m_readPtr    = 0;
         m_eod        = false;
      }
   }

   return retVal;
}

uint64_t FileDvc::SeekBegin(void)
{
   initialize("", m_loop, false);
   return GetFilePos();
}


// This helper function assumes that it has already been determined that we
// have a valid file and buffer and that we need to read data from the file
// into the buffer.
// This function will fill the buffer starting at offset 0 until
// it is full or we run out of file data.
// Therefore, is is assumed that the file is too big to fit into
// a single buffer.
// This function will set m_eof.
size_t FileDvc::fillDataBuffer(char* dst)
{
   size_t bytes2Xfr;
   size_t bytesXfrd = 0;
   size_t bytesRead;
   bool done = false;

   bytes2Xfr = m_bufBytes;

   while(bytesXfrd < bytes2Xfr && !done)
   {
      bytesRead = fread(dst + bytesXfrd, 1, bytes2Xfr - bytesXfrd, m_pFile);
#if !_WINDOWS
         m_eof = (m_fileBytes ==  ftello(m_pFile));
#else
         m_eof = (m_fileBytes == _ftelli64(m_pFile));
#endif // !_WINDOWS

      if(bytesRead < bytes2Xfr)
      {
         if(m_eof && m_loop)
         {
            rewind(m_pFile);
         }
         else
         {
            // end of file or a failure
            done = true;
         }
      }

      bytesXfrd += bytesRead;

   }// end while

   m_endDataPtr = bytesXfrd;
   return bytesXfrd;
}

uint64_t FileDvc::Read(void* dst, size_t eltSize, uint64_t nElts, const void* src)
{
   size_t bytesXfrd = 0;
   bool done = false;
   size_t bytes2Xfr;
   size_t totalBytes = nElts * eltSize;

   src; // warning

   // are we a going concern?
   if(NULL != m_pFile && m_endDataPtr > 0 && nElts > 0)
   {
      // determine where we are in data, read up to end.
      // If we read to end and still have data to read,
      // read more data from file (if necessary), and read again from
      // beginning of buffer
      if(!m_loop && m_readPtr >= m_endDataPtr && m_eof)
      {
         done = true;
      }

      while(bytesXfrd < totalBytes && !done)
      {
         bytes2Xfr = totalBytes - bytesXfrd;
         // Do we need to read new data?
         if((!m_eof || m_loop) && m_readPtr >= m_endDataPtr)
         {
            // Reset read pointer to 0. Read from file to fill
            // buffer if necessary.
            if(m_fileBytes > m_endDataPtr)
            {
               m_endDataPtr = fillDataBuffer(m_pBuf);
            }

            m_readPtr = 0;
         }

         if(bytes2Xfr > m_endDataPtr - m_readPtr)
         {
            bytes2Xfr = m_endDataPtr - m_readPtr;
         }

         memcpy((void*)((char*)dst + bytesXfrd), m_pBuf + m_readPtr, bytes2Xfr);
         bytesXfrd += bytes2Xfr;
         m_readPtr += bytes2Xfr;

         // Stop if we've reached end of data
         if(!m_loop && m_readPtr >= m_endDataPtr && m_eof)
         {
            m_eod = done = true;
         }
      }// end while
      m_filePos += bytesXfrd;
      m_filePos %= m_fileBytes;
   }// end if NULL

   return bytesXfrd;
}

uint64_t FileDvc::Write(const void* src, size_t eltSize, uint64_t nElts, void* dst)
{
   size_t retVal = 0;
   // not implemented for this device.
   src; eltSize; nElts; dst;
   return retVal;
}

}// namespace terbit
