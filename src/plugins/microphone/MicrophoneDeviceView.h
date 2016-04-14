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

#include "connector-core/WorkspaceDockWidget.h"

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace terbit
{

class MicrophoneDevice;

class MicrophoneDeviceView : public WorkspaceDockWidget
{
   Q_OBJECT
public:
   MicrophoneDeviceView(MicrophoneDevice* mic);
   ~MicrophoneDeviceView();

   void SetDeviceIndex(int index);
   void SetVolume(double volume);

   void UpdateStartStopImage();

private slots:
   void OnStartStop();
   void OnDeviceSelected(int index);
   void OnVolumeChanged(int value);

private:
   MicrophoneDevice* m_mic;
   QComboBox* m_deviceCombo;
   QSlider* m_volume;
   QPushButton* m_startStop;
   bool m_dataLoading;
};

}
