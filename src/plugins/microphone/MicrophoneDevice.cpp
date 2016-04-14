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

#include <QAudioInput>

//for qml support . . .
//#include <QtQuickWidgets/QQuickWidget>
//#include <QQmlComponent>
//#include <QQmlContext>
//#include <QQuickItem>

#include "MicrophoneDevice.h"
#include "MicrophoneDeviceView.h"
#include "connector-core/Workspace.h"
#include "connector-core/DataSet.h"
#include "tools/Script.h"
#include "tools/DisplayFFT.h"
#include "connector-core/LogDL.h"

namespace terbit
{

MicrophoneDevice::MicrophoneDevice() : Block(), m_state(STATE_IDLE), m_ds(NULL), m_audio(NULL), m_bufferSize(0), m_activeDeviceIndex(0), m_view(NULL)
{
}

MicrophoneDevice::~MicrophoneDevice()
{
   Shutdown();
   if (m_ds)
   {
      GetWorkspace()->DeleteInstance(m_ds->GetAutoId());
      m_ds = NULL;
   }   

   ClosePropertiesView();
}

const BlockIOCategory_t MicrophoneDevice::OUTPUT_MIC = 0;

bool MicrophoneDevice::ShowPropertiesView()
{
   if (m_view)
   {
      GetWorkspace()->WidgetAlert(m_view);
   }
   else
   {
      //classic widget interface
      m_view = new MicrophoneDeviceView(this);

      /*
       * NOTE: for qml support
       *       Load qml into QQuickWidget
      m_view = new WorkspaceDockWidget(this,BuildPropertiesViewName());


       * simple way  .. . lol . . . restricted to loading from file (not string)
      QQuickWidget* qw = new QQuickWidget(m_view);
      QQmlEngine* se = qw->engine();
      GetWorkspace()->ExtendScriptEngine(se);
      MicrophoneDeviceSW* w = new MicrophoneDeviceSW(se,this);
      se->rootContext()->setContextProperty("m_device",w);
      qw->setSource(QUrl("qrc:/MicrophoneDeviceView.qml"));
      qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
      m_view->setWidget(qw);
      connect(m_view,SIGNAL(destroyed()),this,SLOT(OnPropertiesViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);



      //general method, load qml from file or string
      QQuickWidget* qw = new QQuickWidget(m_view);
      QQmlEngine* se = qw->engine();
      GetWorkspace()->ExtendScriptEngine(se);

      MicrophoneDeviceSW* w = new MicrophoneDeviceSW(se,this);
      qw->rootContext()->setContextProperty("m_device",w);

      QQmlComponent* c = new QQmlComponent(se);

      //set from string
      QUrl url;
      //c->setData("import QtQuick 2.0\nText { text: \"Hello world!\" }",url);

      //set from url/file/resource
      url.setUrl("qrc:/MicrophoneDeviceView.qml");
      c->loadUrl(url);


      QQuickItem* qi = qobject_cast<QQuickItem*>(c->create(qw->rootContext()));
      qw->setContent(c->url(),c,qi);

      qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
      m_view->setWidget(qw);
      */

      connect(m_view,SIGNAL(destroyed()),this,SLOT(OnPropertiesViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);
   }
   return true;
}

void MicrophoneDevice::ClosePropertiesView()
{
   if (m_view)
   {
      m_view->close();
      m_view = NULL;
   }
}

QString MicrophoneDevice::BuildPropertiesViewName()
{
   return QString(tr("%1 Control").arg(GetName()));
}

bool MicrophoneDevice::InteractiveInit()
{   
   ShowPropertiesView();
   m_ds->ShowDisplayView();
   Start();
   return true;
}


bool MicrophoneDevice::Init()
{
   bool res = true;

   QString defaultDeviceName = QAudioDeviceInfo::defaultInputDevice().deviceName();
   m_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
   m_activeDeviceIndex = 0;
   for(int i = 0; i < m_devices.size(); ++i)
   {
      QString deviceName = m_devices.at(i).deviceName();
      m_deviceNames.push_back(deviceName);
      if (defaultDeviceName == deviceName)
      {
         m_activeDeviceIndex = i;
      }
   }

   m_ds = GetWorkspace()->CreateDataSet(this);
   m_ds->SetName(GetName());
   m_ds->CreateBuffer(TERBIT_INT16, 0, 1024);
   m_ds->SetDisplayViewTypeName(TERBIT_TYPE_XYPLOT);
   AddOutput(OUTPUT_MIC, m_ds);

   m_volume = 0.05;
   m_bufferSize = m_ds->GetAllocatedByteCount();

   connect(&m_rawBuffer,SIGNAL(readyRead()), this, SLOT(OnRawBufferReady()));

   return res;
}

void MicrophoneDevice::SetName(const QString &name)
{
   Block::SetName(name);
   if (m_view)
   {
      m_view->setWindowTitle(BuildPropertiesViewName());
   }
}

bool MicrophoneDevice::SetDevice(const QString &deviceName)
{
   bool res = false;
   for(int i = 0; i < m_devices.size(); ++i)
   {
      if  (deviceName == m_devices.at(i).deviceName())
      {
         res = SetDeviceIndex(i);
         break;
      }
   }
   return res;
}

bool MicrophoneDevice::SetDeviceIndex(int index)
{
   bool res = false;
   if (index >= 0 && index < m_devices.size())
   {
      if (m_state == STATE_RUNNING)
      {
         Shutdown();
         m_activeDeviceIndex = index;
         Start();
      }
      else
      {
         m_activeDeviceIndex = index;
      }
      if (m_view)
      {
         m_view->SetDeviceIndex(m_activeDeviceIndex);
      }
      res = true;
   }

   return res;
}

QString MicrophoneDevice::GetDeviceName()
{
   QString res;
   if (m_activeDeviceIndex < m_devices.count())
   {
      res = m_devices.at(m_activeDeviceIndex).deviceName();
   }
   return res;
}

void MicrophoneDevice::SetVolume(double value)
{
   m_volume = value;
   if (m_audio)
   {
      m_audio->setVolume(m_volume);
   }

   if (m_view)
   {
      m_view->SetVolume(m_volume);
   }
}

void MicrophoneDevice::OnPropertiesViewClosed()
{
   m_view = NULL;
}

bool MicrophoneDevice::Shutdown()
{
   if (m_audio)
   {
      m_audio->stop();
      delete m_audio;
      m_audio = NULL;
      m_rawBuffer.close();
      m_state = STATE_IDLE;

      if (m_view)
      {
         m_view->UpdateStartStopImage();
      }
   }
   return true;
}


bool MicrophoneDevice::Start()
{
   bool res = true;

   if (m_audio == NULL)
   {

      QAudioDeviceInfo info;

      if (m_activeDeviceIndex < m_devices.count())
      {
         info = m_devices.at(m_activeDeviceIndex);
      }

      if (info.isNull())
      {
         res = false;
      }
      else
      {
         QAudioFormat format;

         if (!BuildRequestFormat(format))
         {
            //try for something and adjust data set buffer accordingly . . .
            format.setSampleSize(8);
            format.setSampleType(QAudioFormat::UnSignedInt);
            format = info.nearestFormat(format);
            res = AdjustDataSetToFormat(format);
         }
         else if (!info.isFormatSupported(format))
         {
            QAudioFormat requestedFormat = format;
            format = info.nearestFormat(requestedFormat);

            //if we don't get sample size or type we want then need to adjust data set buffer . . .
            if (requestedFormat.sampleSize() != format.sampleSize() || requestedFormat.sampleType() != format.sampleType())
            {
               res = AdjustDataSetToFormat(format);
            }
         }

         if (res)
         {
            m_audio = new QAudioInput(format, this);
            m_audio->setVolume(m_volume);

            m_rawBuffer.open(QBuffer::ReadWrite);
            m_audio->start(&m_rawBuffer);
            m_state = STATE_RUNNING;
            if (m_view)
            {
               m_view->UpdateStartStopImage();
            }

         }
      }
   }

   return res;
}

bool MicrophoneDevice::BuildRequestFormat(QAudioFormat& format)
{
   bool res = true;

   // Set up the desired format, for example:
   format.setSampleRate(8000);
   format.setChannelCount(1);
   format.setCodec("audio/pcm");
   format.setByteOrder(QAudioFormat::LittleEndian);

   switch (m_ds->GetDataType())
   {
   case TERBIT_INT16:
      format.setSampleSize(16);
      format.setSampleType(QAudioFormat::SignedInt);
      break;
   case TERBIT_UINT16:
      format.setSampleSize(16);
      format.setSampleType(QAudioFormat::UnSignedInt);
      break;
   case TERBIT_INT8:
      format.setSampleSize(8);
      format.setSampleType(QAudioFormat::SignedInt);
      break;
   case TERBIT_UINT8:
      format.setSampleSize(8);
      format.setSampleType(QAudioFormat::UnSignedInt);
      break;
   case TERBIT_INT32:
      format.setSampleSize(32);
      format.setSampleType(QAudioFormat::SignedInt);
      break;
   case TERBIT_UINT32:
      format.setSampleSize(32);
      format.setSampleType(QAudioFormat::UnSignedInt);
      break;
   case TERBIT_INT64:
      format.setSampleSize(64);
      format.setSampleType(QAudioFormat::SignedInt);
      break;
   case TERBIT_UINT64:
      format.setSampleSize(64);
      format.setSampleType(QAudioFormat::UnSignedInt);
      break;
   case TERBIT_FLOAT:
      format.setSampleSize(32);
      format.setSampleType(QAudioFormat::Float);
      break;
   case TERBIT_DOUBLE:
      format.setSampleSize(64);
      format.setSampleType(QAudioFormat::Float);
      break;
   default:
      //LogWarning(g_logMicrophone, tr("Unsupported request data type format: %1").arg(m_bufferInfo.type));
      res = false;
   }

   return res;
}

bool MicrophoneDevice::AdjustDataSetToFormat(const QAudioFormat& format)
{
   bool res = true;

   TerbitDataType dataType;

   switch(format.sampleSize())
   {
   case 8:
      switch(format.sampleType())
      {
      case QAudioFormat::SignedInt:
         dataType = TERBIT_INT8;
         break;
      case QAudioFormat::UnSignedInt:
         dataType = TERBIT_UINT8;
         break;
      default:
         res = false;
      };
      break;
   case 16:
      switch(format.sampleType())
      {
      case QAudioFormat::SignedInt:
         dataType = TERBIT_INT16;
         break;
      case QAudioFormat::UnSignedInt:
         dataType = TERBIT_UINT16;
         break;
      default:
         res = false;
      };
      break;
   case 32:
      switch(format.sampleType())
      {
      case QAudioFormat::SignedInt:
         dataType = TERBIT_INT32;
         break;
      case QAudioFormat::UnSignedInt:
         dataType = TERBIT_UINT32;
         break;
      case QAudioFormat::Float:
         dataType = TERBIT_FLOAT;
         break;
      default:
         res = false;
      };
      break;
   case 64:
      switch(format.sampleType())
      {
      case QAudioFormat::SignedInt:
         dataType = TERBIT_INT64;
         break;
      case QAudioFormat::UnSignedInt:
         dataType = TERBIT_UINT64;
         break;
      case QAudioFormat::Float:
         dataType = TERBIT_DOUBLE;
         break;
      default:
         res = false;
      };
      break;
   default:
      res = false;
   };

   if (res)
   {
      size_t nelts = m_ds->GetCount();
      m_ds->CreateBuffer(dataType, 0, nelts);
      m_bufferSize = m_ds->GetAllocatedByteCount();
   }

   return res;
}

void MicrophoneDevice::OnRawBufferReady()
{
   //raw buffer varies in size and data comes in at different amounts each time
   //pos in raw buffer means everything in raw buffer before pos is new data to be read
   //the size of raw buffer can vary and may be extended as needed
   //raw buffer bytesAvailable is the bytes available for writing (not reading)

   //so we wait until raw buffer has enough data for us
   //then we grab data ready to read from raw buffer and copy it to the data set
   //we shift down any leftover data in the raw buffer so we don't miss anything

   if (m_rawBuffer.pos()>m_bufferSize)
   {
      qint64 leftOvers = m_rawBuffer.pos();
      m_rawBuffer.seek(0);
      char* buf = (char*)m_ds->GetBufferAddress();
      qint64 read = m_rawBuffer.read(buf,m_bufferSize);
      leftOvers -= read;

      if (leftOvers > 0)
      {
         char* raw = m_rawBuffer.buffer().data()+m_rawBuffer.pos();
         m_rawBuffer.seek(0);
         m_rawBuffer.write(raw,leftOvers);
      }
      else
      {
         m_rawBuffer.seek(0);
      }

      //ensure the sampling rate is up to date
      m_ds->GetProperties()[TERBIT_DATA_PROPERTY_SAMPLING_RATE] = m_audio->format().sampleRate();
      m_ds->GetProperties()[TERBIT_DATA_PROPERTY_SAMPLING_BITS] = m_audio->format().sampleSize();

      m_ds->SetHasData(true);
      emit m_ds->NewData(m_ds);
   }
}

QObject *MicrophoneDevice::CreateScriptWrapper(QJSEngine* se)
{
   return new MicrophoneDeviceSW(se,this);
}

void MicrophoneDevice::BuildRestoreScript(ScriptBuilder &script, const QString &variableName)
{
   QString name = GetDeviceName();
   script.add(QString("%1.SetDevice(%2);").arg(variableName).arg(ScriptEncode(name)));
   script.add(QString("%1.SetVolume(%2);").arg(variableName).arg(GetVolume()));

   if (m_view)
   {
      script.add(QString("%1.ShowPropertiesWindow();").arg(variableName));
   }
}

ScriptDocumentation *BuildScriptDocumentationMicrophone()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("A basic microphone device."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetDevice"), "SetDevice(deviceName);",QObject::tr("Set the microphone device to used based on installed microphone devices on the OS.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetVolume"), "SetVolume(volume);",QObject::tr("Set the volume.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetVolume"), "GetVolume();",QObject::tr("Set boolean option to loop back to the beginning of the file.  When set to false, the device stops playing when the end of file is reached.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Start"), "Start();",QObject::tr("Starts reading data from the microphone device.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Stop"), "Stop();",QObject::tr("Stops reading data from the microphone device.")));

   return d;
}
MicrophoneDeviceSW::MicrophoneDeviceSW(QJSEngine* se, MicrophoneDevice *mic) : BlockSW(se, mic), m_scriptEngine(se), m_mic(mic)
{
}

void MicrophoneDeviceSW::SetDevice(const QString &deviceName)
{
   m_mic->SetDevice(deviceName);
}

void MicrophoneDeviceSW::SetVolume(double value)
{
   m_mic->SetVolume(value);
}

double MicrophoneDeviceSW::GetVolume()
{
   return m_mic->GetVolume();
}

void MicrophoneDeviceSW::Start()
{
   m_mic->Start();
}

void MicrophoneDeviceSW::Stop()
{
   m_mic->Shutdown();
}

}
