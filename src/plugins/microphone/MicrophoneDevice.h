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
#include <QAudio>
#include <QAudioDeviceInfo>
#include <QBuffer>
#include <list>
#include <stdint.h>
#include "connector-core/Block.h"
#include "connector-core/DataSource.h"
#include "MicrophoneDeviceView.h"
#include "tools/Tools.h"

QT_FORWARD_DECLARE_CLASS(QAudioInput)

namespace terbit
{

class MicrophoneDeviceView;

static const char* MICROPHONEDEVICE_TYPENAME = "microphone";

class MicrophoneDevice : public Block
{
   Q_OBJECT

   friend class MicrophoneDeviceSW;
public:
   MicrophoneDevice();
   ~MicrophoneDevice();

   static const BlockIOCategory_t OUTPUT_MIC;

   bool ShowPropertiesView();
   void ClosePropertiesView();

   QString BuildPropertiesViewName();

   bool InteractiveInit();
   bool Init();
   bool Shutdown();
   QObject* CreateScriptWrapper(QJSEngine* se);
   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   void SetName(const QString &name);

   bool Start();

   const std::list<QString>& GetDeviceList() { return m_deviceNames; }
   bool SetDevice(const QString& deviceName);
   bool SetDeviceIndex(int index);
   QString GetDeviceName();

   double GetVolume() { return m_volume; }
   void SetVolume(double value);

   int GetDeviceIndex() { return m_activeDeviceIndex; }


   enum State
   {
      STATE_IDLE,
      STATE_RUNNING
   };
   State GetState() { return m_state; }

private slots:
   void OnRawBufferReady();
   void OnPropertiesViewClosed();

private:

   bool AdjustDataSetToFormat(const QAudioFormat& format);
   bool BuildRequestFormat(QAudioFormat& format);

   State m_state;
   DataSet* m_ds;
   size_t m_bufferSize;
   QAudioInput* m_audio;
   QBuffer m_rawBuffer;
   QList<QAudioDeviceInfo> m_devices;
   std::list<QString> m_deviceNames;
   int m_activeDeviceIndex;
   double m_volume;
   MicrophoneDeviceView* m_view;
};

//Script Wrapper
ScriptDocumentation *BuildScriptDocumentationMicrophone();

class MicrophoneDeviceSW : public BlockSW
{
   Q_OBJECT

public:
   MicrophoneDeviceSW(QJSEngine* se, MicrophoneDevice* mic);
   virtual ~MicrophoneDeviceSW() {}

   Q_INVOKABLE void SetDevice(const QString& deviceName);
   Q_INVOKABLE void SetVolume(double value);
   Q_INVOKABLE double GetVolume();
   Q_INVOKABLE void Start();
   Q_INVOKABLE void Stop();

private:
   QJSEngine* m_scriptEngine;
   MicrophoneDevice* m_mic;
};

}
