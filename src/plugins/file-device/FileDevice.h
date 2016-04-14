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
#include <QtPlugin>
#include <stdint.h>
#include "connector-core/Block.h"
#include "connector-core/DataSource.h"
#include "tools/Tools.h"
#include "tools/device/filedvc/filedvc.h"
#include <string>

namespace terbit
{

class FileDeviceView;

typedef enum
{
   FDCUnknown,
   FDCRun,
   FDCPause,
   FDCStop,
   FDCTerm
}FileDvcDPCmd_t;

typedef enum
{
   FDSOk,
   FDSErr,
   FDSErrFile,
   FDSErrParam,
   FDSErrInit,
   FDSErrAlloc,
   FDSUnknown
}FileDvcDPStatus_t;

typedef enum
{
   FDMInitializing,
   FDMInitialized,
   FDMStopped,
   FDMRunning,
   FDMPaused,
   FDMTermd
}FileDvcDPMode_t;

static const char* FILE_DATA_STR = "File Data";
static const char* FILE_DEVICE_TYPENAME = "file-device";

class FileDevice : public Block
{
   Q_OBJECT

   friend class FileDeviceSW;

public:
   FileDevice();
   ~FileDevice();

   typedef enum {FileData}FileDvcData_t;

   // ----------- DataClass/Device Interface --------------
   bool Init();
   bool InteractiveInit();
   bool InteractiveFocus();
   bool ShowPropertiesView();
   void ClosePropertiesView();
   void SetName(const QString &name);
   bool ImpendingDeletion();
   QObject* CreateScriptWrapper(QJSEngine* se);
   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   // DataProvider functions to implement/enhance
   bool Shutdown(void);

protected:
   QWidget* CreatePropertiesWidget(QWidget* parent);

public:


   // ----------------- GUI interface-------------------
   // Accessor Functions
   void SetFreq(double f);
   void SetLoop(bool loop);
   void SetNumCh(size_t n);
   void SetDataType(TerbitDataType t);
   //void SetFilePathName(const QString& p){m_filePathName = p;}
   void SetNumElts(uint64_t n);

   double GetFreq(void){return m_freqHz;}
   bool   GetLoop(void){return m_loop;}
   size_t GetNumCh(void){return m_nCh;}
   TerbitDataType   GetDataType(void){return m_dataType;}
   const QString&    GetFilePathName(void){return m_filePathName;}
   uint64_t          GetNumElts(void){return m_nEltsPerBuf;}
   FileDvcDPStatus_t GetStatus(void);
   FileDvcDPMode_t   GetMode(void) {return m_mode;}
   uint64_t          GetFileBytes(void);
   uint64_t          GetFilePos(void);

   // -------------------- Device control ------------------
   bool Start(void);
   bool Stop(void);
   bool Pause(void);
   bool Single(void);

   bool UpdateFile(const QString& p);
   bool UpdateSrc(size_t nElts, TerbitDataType type);
   bool UpdateSrc(size_t nElts);
   bool UpdateSrc(TerbitDataType type);
   void SkipBytes(size_t n);

   // ---------------- Test functions ----------------
   // DO NOT USE these in application code
   int TEST_GetSrcKey(){return 0;}
   bool TEST_GetSingleShot(){return m_singleShot;}

signals:
   void ConfigUpdated(void);

private slots:
   void OnPropertiesViewClosed();

private:
   void processLoop(void);
   bool registerSource(void* pBuf, size_t strideBytes, size_t nElts, QString& str);
   bool initSource(void);
   bool terminate(void);
   void freqSleep(double hz);
   void pauseForUpdate();

   double   m_freqHz     = 10;
   bool     m_loop       = false;
   bool     m_singleShot = false;
   size_t               m_nCh = 1;
   TerbitDataType      m_dataType = TERBIT_UINT8;
   QString              m_filePathName;
   terbit::FileDvc*     m_pFile      = NULL;
   FileDvcDPStatus_t    m_fileStatus = FDSUnknown;
   FileDvcDPStatus_t    m_bufStatus  = FDSUnknown;
   FileDvcDPCmd_t       m_cmd        = FDCUnknown;
   FileDvcDPMode_t      m_mode       = FDMInitializing;
   char*                m_pRaw       = NULL;
   uint64_t             m_nEltsPerBuf;     // requested buffer size
   size_t               m_bufferSizeBytes; // actual buffer size
   boost::thread*       m_outputThread  = NULL;
   bool                 m_threadActive  = false;
   bool                 m_enable        = false;
   // proc-GUI process synchronization
   bool                 m_updateSrc   = false;
   bool                 m_updateFile  = false;
   bool                 m_procWaiting = false;
   size_t               m_skipBytes   = 0;
   DataSet               *m_buf        = NULL;
   FileDeviceView      *m_view       = NULL;

};

ScriptDocumentation *BuildScriptDocumentationFileDevice();

//Script Wrapper
class FileDeviceSW : public BlockSW
{
   Q_OBJECT

public:
   FileDeviceSW(QJSEngine* , FileDevice* fileDvc);
   virtual ~FileDeviceSW() {}

   Q_INVOKABLE void SetFreq(double f);
   Q_INVOKABLE void SetLoop(bool loop);
   Q_INVOKABLE bool SetDataType(int t);
   Q_INVOKABLE bool SetFilePathName(const QString& p);
   Q_INVOKABLE QString GetFilePathName();
 #ifdef TERBIT_32BIT
   Q_INVOKABLE bool SetNumElts(quint32 n);
 #else
   Q_INVOKABLE bool SetNumElts(quint64 n);
#endif

   Q_INVOKABLE bool Start();
   Q_INVOKABLE bool Pause();
   Q_INVOKABLE bool Stop();
   Q_INVOKABLE bool Single();

private:
   FileDevice* m_fileDvc;
};



}// namespace terbit
