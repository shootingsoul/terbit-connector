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

#include "stdint.h"
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>

QT_BEGIN_NAMESPACE
class QPushButton;
class QGridLayout;
QT_END_NAMESPACE

namespace terbit
{


class FileDeviceViewAdvanced : public QWidget
{
   Q_OBJECT
public:
   explicit FileDeviceViewAdvanced(QWidget *parent = 0);
   void SetBufSizeEnabled(bool en){m_pBufSize->setEnabled(en);}
   void SetFreqSpinEnabled(bool en){m_pSpinFreq->setEnabled(en);}
   void SetDataTypeEnabled(bool en){m_pComboType->setEnabled(en);}
   QSpinBox* GetBufSizeBox(void){return m_pBufSize;}
   QDoubleSpinBox* GetFreqBox(void){return m_pSpinFreq;}
   QComboBox* GetDataTypeBox(void){return m_pComboType;}

private slots:
   void expandAdv();
   void byteAdjustBtn();

signals:
   void ByteAdjust();

private:
   void setActions();

   QPushButton    *m_pBtnAdvanced;
   QPushButton    *m_pBtnByteAdj;
   QDoubleSpinBox *m_pSpinFreq;
   QSpinBox       *m_pBufSize;
   QGridLayout    *m_pLayout;
   QComboBox      *m_pComboType;
   QWidget        *m_optionsDlg;
   bool            m_AdvExpanded;
   QWidget        *m_parent;
};

}
