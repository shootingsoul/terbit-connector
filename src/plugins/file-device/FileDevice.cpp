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

/*! The FileDevice class implements a file player functionality, minus the GUI.
 * Also minus the file IO.  File IO is implemented in the filedvc class, which
 * handles the lower level file, buffer, and wrap-around details.  Each
 * FileDevice instance will have at most one instance of a filedvc class at
 * any one time, although multiple filedvc instances can be created and
 * destroyed over the lifetime of a FileDevice instance.
 * Each FileDevice instance registers one dataset with workspace.
 * Each instance of the class spawns a thread that will call for data updates
 * at the configured frequency.  The FileDevice class provides a set/get
 * interface for a GUI.
 * The FileDevice class implements a state machine in the thread to ensure
 * correct behaviour based on modes, status, and commands.  The modes are:
 * o FDMInitializing - This is the mode during construction and early init.
 *    This mode then changes to FDMInitialized once the thread starts running.
 * o FDMInitialized - This mode is set once the thread is running.
 * o FDMStopped - This mode is set when data transmission is stopped by use
 *    of the FDCStop command, or by completing a file transmission.
 * o FDMRunning - This mode is set when data transmission is in progress.  We
 *    normally arrive here via the FDCRun command or via the "Single Buffer"
 *    functionality.
 * o FDMPaused - This mode is set when data transmission is "paused".  We get
 *    here via the FDCPause command.
 * o FDMTermd - This mode is set by the thread when it exits.  This means
 *    that no more states will be processed, nor data transmitted.
 * The commands and statuses that drive the state machine transitions are
 * pretty straightforward.  Refer to the code for more detail.  More detail
 * is typically not required as the file player will be controlled by the
 * following functions:
 * o Init()   - Start the processing thread, returning status
 * o Start()  - begin transmission of data, reconfigure first if necessary
 * o Stop()   - halt transmission of data, reset file back to beginning.
 * o Pause()  - pause transmission of data, keep location in file
 * o Single() - transmit one buffer of data, reconfigure first if necessary
 * The class provides several configuration options:
 * o Buffer size in number of elements - this option is staged until the next
 *   stop->start transition.
 * o Data type of input data - this option can take effect immediately.
 * o File name - This option is staged until the next stop->start transition.
 * o Data buffer transmission frequency.  Note that very low frequencies
 *   could cause the processing thread to sleep for many minutes.  The
 *   sleep code is therefore implemented in a helper function that breaks
 *   long sleeps into multiple sleeps of a maximum allowable time (~1s).
 *   This loop will break if a command such as stop, pause, or single is
 *   received.  This option can take effect immediately.
 * o Loop - whether to loop the file input continuously.  This option can
 *   be set immediately.
 * These configuration items have accessor functions for the GUI.  Another
 * item available to the GUI is "FilePos", which is the current file pointer
 * position.  This allows the GUI to show progress through a file.
 *
 * ----------------------------- Object Creation ----------------------------
 * The default construction initializes many member variables, but does not
 * initialize default values and start the processing thread.  In order to do
 * that, the Init() function must be called.
 * Further initialization is performed by InteractiveInit(), which is called
 * as a result of GUI actions
 *
 * --------------------------- Thread Safety Issues -------------------------
 * Data Source (DataSet) configuration must be done in the "GUI thread".  Not
 * in the processing thread.  Therefore, variables functions are provided to
 * synchronize the GUI and processing thread when performing configuration
 * functions such as UpdateFile() and UpdateSrc(). DO NOT call these functions
 * from the processing thread.  The function pauseForUpdate() is called by the
 * processing thread and will synchronize if either of two variable are set.
 * The two variables are m_updateSrc and m_updateFile.  The boolean variable
 * m_procWaiting is set by the processing thread to tell the GUI thread
 * that it is waiting (safe to proceed).
 *
 * Inter-thread communication within this class (such as in UpdateFile() or
 * UpdateSrc() is protected against "locking up" by implementing timeouts.
 *
 * ------------------------ Future Features -----------------------
 * o Multiple channels (each with its own data source) within a single file.
 * o Multiple files - a sequence of files instead of just a single file.
 *   The file management portion of this should be implemented in the FileDvc
 *   class.
 *
 * NOTE: certain functions use a uint64_t type instead of size_t so
 *       that 32-bit builds can access large files.  size_t would resolve
 *       to 32-bits on a 32-bit build.
 **************************************************************************/
//#include <QtConcurrent/QtConcurrent>
#include <QFileInfo>
#include "FileDevice.h"
#include "FileDeviceView.h"
#include "connector-core/Workspace.h"
#include "connector-core/DataSet.h"
#include "tools/Script.h"
#include "connector-core/LogDL.h"

namespace terbit
{

static const uint32_t FD_THREAD_TIMEOUT_MS = 500;
static const double   MIN_FREQ_SLEEP_HZ    = 1.0;
static const size_t   START_NELTS          = 2048;

static void convertToDouble(char* src, double* dst, size_t nDstElts, TerbitDataType t);

FileDevice::FileDevice()
{
   m_nEltsPerBuf     = START_NELTS;
   m_bufferSizeBytes = m_nEltsPerBuf * TerbitDataTypeSize(TERBIT_DOUBLE);
}


FileDevice::~FileDevice()
{
   m_cmd = FDCTerm;
   if(!Shutdown())
   {
      // TODO: message that we couldn't terminate the process
   }

   ClosePropertiesView();

   //delete m_outputThread;
   m_outputThread = NULL;
   delete[] m_pRaw;
   delete m_pFile;
}

void FileDevice::SetName(const QString &name)
{
   Block::SetName(name);
   if (m_view)
   {
      m_view->setWindowTitle(name);
   }
}

bool FileDevice::ImpendingDeletion()
{
   return Shutdown();

}
void FileDevice::OnPropertiesViewClosed()
{
   m_view = NULL;
}


// This function should be called only once for the lifetime of
// the FileDevice instance.  This function fires up the processing thread.
bool FileDevice::Init()
{

   SetFreq(10.000);
   SetLoop(false);
   SetNumCh(0);
   SetDataType(TERBIT_INT16);
   SetNumElts(START_NELTS);
#if 0
   if(!m_threadActive)
   {
      QtConcurrent::run(this, &FileDevice::processLoop);
      m_threadActive = true;
   }
#else
   if(!m_threadActive)
   {
      m_outputThread = new(std::nothrow) boost::thread(boost::bind(&FileDevice::processLoop, this));
      if(NULL != m_outputThread)
      {
         m_outputThread->try_join_for(boost::chrono::milliseconds(FD_THREAD_TIMEOUT_MS));
      }
   }
#endif
   return m_threadActive && initSource();
}

bool FileDevice::InteractiveInit()
{
   return ShowPropertiesView();
}

bool FileDevice::InteractiveFocus()
{
   return ShowPropertiesView();
}


bool FileDevice::Shutdown(void)
{
   bool term = terminate();

   if(m_buf)
   {
      RemoveOutput(FileData);
      GetWorkspace()->DeleteInstance(m_buf->GetAutoId());
      m_buf = NULL;
   }

   if(!term)
   {
      m_outputThread->detach();
   }
   return term;
}


bool FileDevice::initSource()
{
   bool retVal = false;
   m_bufferSizeBytes = m_nEltsPerBuf * TerbitDataTypeSize(TERBIT_DOUBLE);

   m_buf = GetWorkspace()->CreateDataSet(this);
   if(NULL != m_buf)
   {
      m_buf->SetName(GetName());
      m_buf->CreateBuffer(m_dataType, 0, m_nEltsPerBuf);
      m_buf->SetDisplayViewTypeName(TERBIT_TYPE_XYPLOT);
      AddOutput(FileData, m_buf);

      m_bufStatus = FDSOk;
      retVal = true;
   }
   else
   {
      m_bufStatus = FDSErrAlloc;
      ; // TODO: log error
   }

   return retVal;
}

uint64_t FileDevice::GetFileBytes(void)
{
   uint64_t retVal = 0;

   if(m_pFile && m_pFile->IsValid())
   {
      retVal = m_pFile->GetFileBytes();
   }
   return retVal;
}


uint64_t FileDevice::GetFilePos(void)
{
   uint64_t retVal = 0;
   if(m_pFile && m_pFile->IsValid())
   {
      retVal = m_pFile->GetFilePos();
   }
   return retVal;
}


bool FileDevice::ShowPropertiesView()
{
   bool retVal = true;
   if(m_view)
   {
      GetWorkspace()->WidgetAlert(m_view);
   }
   else
   {

      m_view =  new(std::nothrow) FileDeviceView(this);
      connect(m_view, SIGNAL(destroyed()), this, SLOT(OnPropertiesViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);
      retVal = true;
   }
   return retVal;
}

void FileDevice::ClosePropertiesView()
{
   if (m_view)
   {
      m_view->close();
      m_view = NULL;
   }
}


bool FileDevice::Start()
{
   int32_t timeout = 25;

   m_cmd = FDCRun;

   while(timeout > 0 && m_mode != FDMRunning)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
   }

   return m_mode == FDMRunning;
}


bool FileDevice::Single()
{
  m_singleShot = true;
  // Start could return false if it sleeps during the transition
  // through mode FDMRunning.  If it set singleShot back to false, then
  // we'll consider that a successful single command.
  return Start() || !m_singleShot;
}


bool FileDevice::Stop(void)
{
   int32_t timeout = 25;

   m_cmd = FDCStop;

   while(timeout > 0&& m_mode != FDMStopped)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
   }

   return m_mode == FDMStopped;
}


bool FileDevice::Pause(void)
{
   int32_t timeout = 25;

   m_cmd = FDCPause;

   while(timeout > 0 && m_mode != FDMPaused)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
   }

   return m_mode == FDMPaused;
}


void FileDevice::SetLoop(bool loop)
{
   m_loop = loop;
   if(m_pFile)
   {
      m_pFile->SetLoop(loop);
      emit ConfigUpdated();
   }
}

void FileDevice::SetFreq(double f)
{
   if(f>0)
   {
      m_freqHz = f;
      emit ConfigUpdated();
   }
}


void FileDevice::SetNumCh(size_t n)
{
   if(n > 0)
   {
      m_nCh = n;
      emit ConfigUpdated();
   }
}

void FileDevice::SetDataType(TerbitDataType t)
{
   if(t >= TERBIT_INT8 && t <= TERBIT_DOUBLE)
   {
      m_dataType = t;
      emit ConfigUpdated();
   }
}

void FileDevice::SetNumElts(uint64_t n)
{
   if(n > 0)
   {
      m_nEltsPerBuf = n;
      emit ConfigUpdated();
   }
}


bool FileDevice::terminate(void)
{
   int32_t timeout = 25;

   m_cmd = FDCTerm;

   while(timeout > 0&& m_mode != FDMTermd && NULL != m_outputThread)
   //while(timeout > 0&& m_mode != FDMTermd)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
   }

   return m_mode == FDMTermd || NULL == m_outputThread;
   //return m_mode == FDMTermd;
}


void FileDevice::pauseForUpdate()
{
   int32_t timeout = 25;

   if(m_updateSrc)
   {
      m_procWaiting = true;
      while(timeout > 0 && m_updateSrc)
      {
         timeout--;
         boost::this_thread::sleep(boost::posix_time::milliseconds(10));
      }
   }

   if(m_updateFile)
   {
      timeout = 25;
      m_procWaiting = true;
      while(timeout > 0 && m_updateFile)
      {
         timeout--;
         boost::this_thread::sleep(boost::posix_time::milliseconds(10));
      }
   }
   m_procWaiting = false;
}

FileDvcDPStatus_t FileDevice::GetStatus(void)
{
   if(m_fileStatus != FDSOk)
   {
      return m_fileStatus;
   }
   else
   {
      // Return bufStatus whether error or FDSOk.
      // (if OK, then FileDevice status is OK).
      return m_bufStatus;
   }
}

bool FileDevice::UpdateFile(const QString& p)
{
   bool retVal = true;

   int32_t timeout = 100;

   m_updateFile = true;
   while(timeout > 0 && m_procWaiting == false)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
   }

   if(m_procWaiting)
   {
      if(m_pFile)
      {
         delete m_pFile;
         m_pFile = NULL;
      }

      // Try to create new file and buffer based on params
      m_pFile = new(std::nothrow) FileDvc();
      if(m_pFile->Open(p, m_loop) && m_pFile->IsValid())
      {
         QFileInfo fi(p);
         QString str("Terbit.FileDvc.");
         str.append(fi.fileName());
         // TODO: m_svcs->SetDataSetName(m_srcId, str);
         m_filePathName = p;
         m_fileStatus = FDSOk;
      }
      else
      {
         if(NULL != m_pFile)
         {
            delete m_pFile;
            m_pFile = NULL;
         }
         retVal = false;
         m_fileStatus = FDSErrFile;
      }
   }
   else
   {
      retVal = false;
   }
   emit ConfigUpdated();
   m_updateFile = false;
   return retVal;
}

// Sets a variable to tell processing thread that we want to skip a
// certain number of bytes before the next read.  This function is
// not thread safe, because the member variable can be accessed asynch.
// Missing a skip command occassionally is a fair tradeoff against the
// added complexity of mutual exclusion
void FileDevice::SkipBytes(size_t n)
{
   m_skipBytes = n;
}

// helper function to update source when only nElts changed
bool FileDevice::UpdateSrc(size_t nElts)
{
   return UpdateSrc(nElts, m_dataType);
}

// helper furnction update source when only the type changed
bool FileDevice::UpdateSrc(TerbitDataType type)
{
   return UpdateSrc(m_nEltsPerBuf, type);
}

// Call this function from GUI thread to avoid conflicts with
// plotting code.
bool FileDevice::UpdateSrc(size_t nElts, TerbitDataType type)
{
   bool retVal = true;
   int32_t timeout = 100;

   // Sync with processing thread
   m_updateSrc = true;
   while(timeout > 0 && m_procWaiting == false)
   {
      timeout--;
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
   }

   if(m_procWaiting)
   {
      // We can reallocate new buffer here as long as we are running from the
      // GUI thread (graphs won't update).
      if(retVal)
      {
         m_nEltsPerBuf = nElts;
         m_dataType    =  type;
         m_bufStatus   = FDSOk;         
         m_buf->CreateBuffer(m_dataType, 0, m_nEltsPerBuf);
      }
      else
      {
         m_bufStatus = FDSErrInit;
      }
   }
   else
   {
      retVal = false;
   }


   emit ConfigUpdated();

   // Signal processing thread that we're done
   m_updateSrc = false;
   return retVal;
}


void FileDevice::processLoop(void)
{
   bool term = false;
   FileDvcDPCmd_t lastCmd = FDCUnknown;
   size_t xfrBytes = 0;
   uint32_t noActSleepMs = 50;
   //processParams_t locParms;

   m_threadActive = true;
   m_mode = FDMInitialized;
   while (!term)
   {
      pauseForUpdate();
      switch(m_cmd)
      {
      case FDCRun:
         xfrBytes = m_nEltsPerBuf * TerbitDataTypeSize(m_dataType);

         if(FDSOk == m_fileStatus && FDSOk == m_bufStatus)
         {
            m_mode = FDMRunning;
            if(m_skipBytes)
            {
               char* tmpBuf = new(std::nothrow) char[m_skipBytes];
               if(NULL != tmpBuf)
               {
                  m_pFile->Read(tmpBuf, 1, m_skipBytes, NULL);
               }
               delete[] tmpBuf;
               m_skipBytes = 0;
            }
            size_t tmp = m_pFile->Read(m_buf->GetBufferAddress(), 1, xfrBytes, NULL);
            if(xfrBytes != tmp)
            {
               // did we expect to get less?
               if(false == m_loop && m_pFile->GetEof())
               {
                  // Not an error
                  // Try to make it clear that the entire buffer
                  // was not filled
                  // If no data was transferred, leave final buffer alone
                  if(tmp)
                  {
                     memset((char*)(m_buf->GetBufferAddress()) + tmp, 0, xfrBytes - tmp);
                  }
                  m_cmd = FDCStop;
                  m_singleShot = false;
               }
               else
               {
                  // This is an error
                  m_fileStatus = FDSErrFile;
                  m_cmd        = FDCStop;
                  m_singleShot = false;
               }
            }
            else if(false == m_loop && m_pFile->GetEof()) // data ended on buffer boundary
            {
               m_cmd = FDCStop;
               m_singleShot = false;
            }
            // Send data to be graphed.
            m_buf->SetHasData(true);
            emit m_buf->NewData(m_buf);
            if(m_singleShot)
            {
               m_singleShot = false;
               m_cmd = FDCPause;
            }
            else
            {
               freqSleep(m_freqHz);
            }

            emit ConfigUpdated();
         }
         lastCmd = FDCRun;
         break;

      case FDCPause:
         // Only enter pause mode if we come from Run
         if(FDMRunning == m_mode)
         {
            m_mode = FDMPaused;
         }
         //QThread::msleep(noActSleepMs);
         boost::this_thread::sleep(boost::posix_time::milliseconds(noActSleepMs));
         lastCmd = FDCPause;
         break;

      case FDCStop:
         m_mode = FDMStopped;

         if(FDCStop != lastCmd && FDSOk == m_fileStatus)
         {
            if (0 != m_pFile->SeekBegin())
            {
               m_fileStatus = FDSErrFile;
               ;// TODO: log message
            }

            emit ConfigUpdated();
         }
         //QThread::msleep(noActSleepMs);
         boost::this_thread::sleep(boost::posix_time::milliseconds(noActSleepMs));
         lastCmd = FDCStop;
         break;

      case FDCTerm:
         // TODO: unregister data source (and device?) from DataVis
         term = true;
         lastCmd = FDCTerm;
         break;

      case FDCUnknown:
      default:
         lastCmd = FDCUnknown;
         //QThread::msleep(noActSleepMs);
         boost::this_thread::sleep(boost::posix_time::milliseconds(noActSleepMs));
         break;
      }// end switch
   }// end while

   m_mode = FDMTermd;
   m_threadActive = false;
}


// this helper function is called while running to sleep the amount
// required to generate the update frequency.  For long sleeps, this
// function will also return early if a stop or pause command is recieved.
// Also returns early if frequency is changed
void FileDevice::freqSleep(double hz)
{
   double dtemp = hz;
   int32_t i = 1;

   if(hz < MIN_FREQ_SLEEP_HZ)
   {
      i = MIN_FREQ_SLEEP_HZ/hz;
      // Set sleep freq near MinFreqSleep.  Account for rounding error.
      dtemp = hz * i;
   }

   while(FDCRun == m_cmd && i > 0 && hz == m_freqHz)
   {
      boost::this_thread::sleep(boost::posix_time::milliseconds((long)(1000/dtemp)));
      --i;
   }
}


static void convertToDouble(char* src, double* dst, size_t nDstElts, TerbitDataType t)
{
  double* pDst = dst;
  switch(t)
  {
  case TERBIT_INT8:
  {
     int8_t* pSrc = (int8_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_UINT8:
  {
     uint8_t* pSrc = (uint8_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_INT16:
  {
     int16_t* pSrc = (int16_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_UINT16:
  {
     uint16_t* pSrc = (uint16_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_INT32:
  {
     int32_t* pSrc = (int32_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_UINT32:
  {
     uint32_t* pSrc = (uint32_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_INT64:
  {
     int64_t* pSrc = (int64_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_UINT64:
  {
     uint64_t* pSrc = (uint64_t*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
     break;
  }

  case TERBIT_FLOAT:
  {
     float* pSrc = (float*)src;
     for(size_t i = 0; i < nDstElts; ++i)
     {
        pDst[i] = pSrc[i];
     }
  }
     break;

  case TERBIT_DOUBLE:
     memcpy(dst, src, nDstElts * sizeof(double));
     break;

  default:
     break;
  }
}

/*************************** SCRIPT INTERFACE ******************************/
ScriptDocumentation *BuildScriptDocumentationFileDevice()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("Binary file device reader."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetFilePathName"), "SetFilePathName(fileName);",QObject::tr("The fully pathed file name.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetFilePathName"), "GetFilePathName();",QObject::tr("Returns the fully pathed file name.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetFreq"), "SetFreq(freq);",QObject::tr("Set the frequency.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetLoop"), "SetLoop(loop);",QObject::tr("Set boolean option to loop back to the beginning of the file.  When set to false, the device stops playing when the end of file is reached.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetDataType"), "SetDataType(dataType);",QObject::tr("Sets the data type (enum) to use for the binary file data.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetNumElts"), "SetNumElts(value);",QObject::tr("The number of elements to read at a time.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Start"), "Start();",QObject::tr("Starts reading file data.  Returns boolean.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Pause"), "Pause();",QObject::tr("Pauses reading file data.  Returns boolean.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Stop"), "Stop();",QObject::tr("Stops reading file data.  Returns boolean.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Single"), "Single();",QObject::tr("Reads single frame of file data.  Returns boolean.")));

   return d;
}

QObject *FileDevice::CreateScriptWrapper(QJSEngine *se)
{
   return new FileDeviceSW(se, this);
}

void FileDevice::BuildRestoreScript(ScriptBuilder &script, const QString& variableName)
{
   script.add(QString("%1.SetFilePathName(%2);\n").arg(variableName).arg(ScriptEncode(GetFilePathName())));
   script.add(QString("%1.SetFreq(%2);\n").arg(variableName).arg(GetFreq()));
   script.add(QString("%1.SetLoop(%2);\n").arg(variableName).arg(GetLoop()));
   script.add(QString("%1.SetDataType(%2);\n").arg(variableName).arg(GetDataType()));
   script.add(QString("%1.SetNumElts(%2);\n").arg(variableName).arg(GetNumElts()));
   if(m_view)
   {
      script.add(QString("%1.ShowPropertiesWindow();").arg(variableName));
   }
}

FileDeviceSW::FileDeviceSW(QJSEngine *se, FileDevice *fileDvc) : BlockSW(se, fileDvc), m_fileDvc(fileDvc)
{
}
void FileDeviceSW::SetFreq(double f)
{
   m_fileDvc->SetFreq(f);
}

void FileDeviceSW::SetLoop(bool loop)
{
   m_fileDvc->SetLoop(loop);
}

bool FileDeviceSW::SetDataType(int t)
{
   return m_fileDvc->UpdateSrc((TerbitDataType)t);
}

bool FileDeviceSW::SetFilePathName(const QString &p)
{
   if(m_fileDvc->GetMode() == FDMInitialized || m_fileDvc->GetMode() == FDMStopped)
   {
      return m_fileDvc->UpdateFile(p);
   }
   else
   {
      return false;
   }
}

QString FileDeviceSW::GetFilePathName()
{
   return m_fileDvc->GetFilePathName();
}

#ifdef TERBIT_32BIT
bool FileDeviceSW::SetNumElts(quint32 n)
#else
bool FileDeviceSW::SetNumElts(quint64 n)
{
   m_fileDvc->SetNumElts((size_t)n);
   if(m_fileDvc->GetMode() == FDMInitialized || m_fileDvc->GetMode() == FDMStopped)
   {
      return m_fileDvc->UpdateSrc(n);
   }
   else
   {
      return false;
   }
}

bool FileDeviceSW::Start()
{
   return m_fileDvc->Start();
}

bool FileDeviceSW::Pause()
{
   return m_fileDvc->Pause();
}

bool FileDeviceSW::Stop()
{
   return m_fileDvc->Stop();
}

bool FileDeviceSW::Single()
{
   return m_fileDvc->Single();
}

#endif

}// namespace terbit
