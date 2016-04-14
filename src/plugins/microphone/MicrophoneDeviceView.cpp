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
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include "connector-core/ResourceManager.h"
#include "MicrophoneDevice.h"
#include "MicrophoneDeviceView.h"

namespace terbit
{

MicrophoneDeviceView::MicrophoneDeviceView(MicrophoneDevice *mic) : WorkspaceDockWidget(mic, mic->BuildPropertiesViewName()), m_mic(mic), m_dataLoading(false)
{

   m_dataLoading = true;

   m_deviceCombo = new QComboBox();
   std::list<QString>::const_iterator it;
   for(it = m_mic->GetDeviceList().begin(); it != m_mic->GetDeviceList().end(); ++it)
   {
      m_deviceCombo->addItem(*it);
   }
   if (m_deviceCombo->count() > 0)
   {
      m_deviceCombo->setCurrentIndex(m_mic->GetDeviceIndex());
   }
   connect(m_deviceCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(OnDeviceSelected(int)));

   m_startStop = new QPushButton();
   QSize startStopSize;
   int border = 2*m_startStop->style()->pixelMetric(QStyle::PM_ButtonMargin,0,m_startStop);
   startStopSize.setHeight(32+border);
   startStopSize.setWidth(32+border);
   m_startStop->setFixedSize(startStopSize);
   UpdateStartStopImage();
   connect(m_startStop,SIGNAL(clicked()),this,SLOT(OnStartStop()));

   m_volume = new QSlider(Qt::Horizontal,NULL);
   m_volume->setTracking(true);
   m_volume->setMinimum(0);
   m_volume->setMaximum(1000);
   m_volume->setSingleStep(1);
   m_volume->setPageStep(10);
   m_volume->setSliderPosition((int)(m_mic->GetVolume()*m_volume->maximum()));
   connect(m_volume, SIGNAL(valueChanged(int)), this, SLOT(OnVolumeChanged(int)));

   QHBoxLayout* layoutVolume = new QHBoxLayout();
   layoutVolume->addWidget(new QLabel(tr("Volume")));
   layoutVolume->addWidget(m_volume,1);

   QGridLayout* layout = new QGridLayout();
   layout->setColumnStretch(1,1);
   layout->addWidget(m_startStop,0,0);
   layout->addLayout(layoutVolume,0,1);
   layout->addWidget(new QLabel(tr("Device")),1,0);
   layout->addWidget(m_deviceCombo,1,1);

   QWidget* wrapper = new QWidget();
   wrapper->setLayout(layout);
   setWidget(wrapper);

   m_dataLoading = false;
}

MicrophoneDeviceView::~MicrophoneDeviceView()
{
   //handled by event instead of explicit call . . .
   //m_mic->OnPropertiesViewClosed();
}

void MicrophoneDeviceView::UpdateStartStopImage()
{
   if (m_mic->GetState() == MicrophoneDevice::STATE_RUNNING)
   {
      m_startStop->setIcon(ResourceManager::GetPlayerStopImage());
   }
   else
   {
      m_startStop->setIcon(ResourceManager::GetPlayerStartImage());
   }
   m_startStop->setIconSize(QSize(32, 32));
   m_startStop->update();
}

void MicrophoneDeviceView::SetDeviceIndex(int index)
{
   m_dataLoading = true;
   m_deviceCombo->setCurrentIndex(index);
   m_dataLoading = false;
}

void MicrophoneDeviceView::SetVolume(double volume)
{
   m_dataLoading = true;
   m_volume->setSliderPosition((int)(volume*m_volume->maximum()));
   m_dataLoading = false;
}

void MicrophoneDeviceView::OnStartStop()
{
   if (m_mic->GetState() == MicrophoneDevice::STATE_RUNNING)
   {
      m_mic->Shutdown();
   }
   else
   {
      if (!m_mic->Start())
      {
         QMessageBox m(this);
         m.setText(tr("Could not start the microphone."));
         m.exec();
      }
   }
}

void MicrophoneDeviceView::OnDeviceSelected(int index)
{
   if (!m_dataLoading)
   {
      m_mic->SetDeviceIndex(index);
   }
}

void MicrophoneDeviceView::OnVolumeChanged(int value)
{
   if (!m_dataLoading)
   {
      m_mic->SetVolume(value/(double)m_volume->maximum());
   }
}

}

