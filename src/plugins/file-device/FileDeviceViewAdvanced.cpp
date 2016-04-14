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

#include "FileDeviceViewAdvanced.h"
#include "stdint.h"
#include <QtWidgets>
#include "tools/Tools.h"

#define X(a, b) b,
static char *TerbitDataTypeStrs[] = { TERBIT_DATA_TYPES };
#undef X

namespace terbit
{


FileDeviceViewAdvanced::FileDeviceViewAdvanced(QWidget *parent) :
   QWidget(parent)
{
   m_parent = parent;
   m_AdvExpanded  = false;
   m_pBtnAdvanced = new QPushButton;
   m_pBtnByteAdj  = new QPushButton;
   m_pSpinFreq    = new QDoubleSpinBox;
   m_pBufSize     = new QSpinBox;
   m_pLayout      = new QGridLayout(this);
   m_pComboType   = new QComboBox;
   m_optionsDlg   = new QWidget(this);


   QGridLayout* pLayout = new QGridLayout(m_optionsDlg);
   pLayout->addWidget(new QLabel(tr("Update Freqency (Hz)")), 1,0,1,1,0);
   m_pSpinFreq->setDecimals(3);
   m_pSpinFreq->setMaximum(240.0);
   m_pSpinFreq->setMinimum(.01);
   m_pSpinFreq->setValue(10.0);
   m_pSpinFreq->setSingleStep(10.0);
   m_pSpinFreq->setKeyboardTracking(false);
   pLayout->addWidget(m_pSpinFreq, 1, 1, 1, 1, 0);

   QLabel *lbl = new QLabel(tr("n Data points (hex)"));
   lbl->setToolTip(tr("Buffer size in number of elements"));
   pLayout->addWidget(lbl, 2,0,1,1,0);
   m_pBufSize->setMinimum(2);
   m_pBufSize->setMaximum(0x00020000);
   m_pBufSize->setValue(2048);
   m_pBufSize->setSingleStep(2048);
   m_pBufSize->setKeyboardTracking(false);
   m_pBufSize->setDisplayIntegerBase(16);
   m_pBufSize->setToolTip(tr("Buffer size in number of elements (hex)"));
   pLayout->addWidget(m_pBufSize, 2, 1, 1, 1, 0);

   pLayout->addWidget(new QLabel(tr("Type of data in file")), 3,0,1,1,0);
   m_pComboType->setInsertPolicy(QComboBox::InsertAtBottom);
   for(uint32_t i = 0; i < sizeof(TerbitDataTypeStrs)/sizeof(TerbitDataTypeStrs[0]); ++i)
   {
      m_pComboType->addItem(TerbitDataTypeStrs[i], i);
   }
   pLayout->addWidget(m_pComboType, 3, 1, 1, 1, 0);

   m_pBtnByteAdj->setText(tr("Byte Adjust"));
   m_pBtnByteAdj->setToolTip(tr("Discards a single byte before the next read"));
   m_pBtnByteAdj->setWhatsThis(tr("Use this button if you suspect a data alignment issue.  It will drop a single byte for each click (effectively shifting the pointer right one byte)"));
   pLayout->addWidget(m_pBtnByteAdj, 4, 1, 1, 1, 0);

   m_pBtnAdvanced->setStyleSheet("text-align: right");
   m_pBtnAdvanced->setText(tr("Advanced   >>>"));
   m_pBtnAdvanced->setToolTip(tr("Show or hide advanced options"));
   m_pLayout->addWidget(m_pBtnAdvanced, 0, 0, 1, 1, 0);

   m_pLayout->addWidget(m_optionsDlg, 1, 0, 1, 1, 0);
   setLayout(m_pLayout);   
   m_optionsDlg->hide();

   setTabOrder(m_pBtnAdvanced, m_pSpinFreq);
   setTabOrder(m_pSpinFreq, m_pBufSize);
   setTabOrder(m_pBufSize, m_pComboType);

   setActions();

}

void FileDeviceViewAdvanced::setActions()
{
   connect(m_pBtnAdvanced, SIGNAL(clicked()), this, SLOT(expandAdv()));
   connect(m_pSpinFreq,    SIGNAL(valueChanged(double)), m_parent, SLOT(OnFreq()));
   connect(m_pComboType,   SIGNAL(currentIndexChanged(int)), m_parent, SLOT(OnDataType()));
   connect(m_pBufSize,   SIGNAL(valueChanged(int)), m_parent, SLOT(OnBufSize()));
   connect(m_pBtnByteAdj, SIGNAL(clicked()), this, SLOT(byteAdjustBtn()));
}

void FileDeviceViewAdvanced::expandAdv(void)
{
   m_AdvExpanded = !m_AdvExpanded;

   if(m_AdvExpanded)
   {
      m_pBtnAdvanced->setText(tr("Advanced   <<<"));
      m_optionsDlg->show();
   }
   else
   {
      m_pBtnAdvanced->setText(tr("Advanced   >>>"));
      m_optionsDlg->hide();
   }
}

void FileDeviceViewAdvanced::byteAdjustBtn()
{
   emit ByteAdjust();
}

}
