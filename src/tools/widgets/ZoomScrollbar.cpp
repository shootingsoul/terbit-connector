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
#include "ZoomScrollbar.h"
#include "ZoomScrollbarSlider.h"
#include "ScrollbarButton.h"
#include "tools/Tools.h"
#include <stdint.h>
#include <math.h>
#include <QBoxLayout>
#include <QPushButton>
#include <QDateTime>
#include <QStyle>
#include <QPainter>


namespace terbit
{

ZoomScrollbar::ZoomScrollbar(QWidget* parent, Qt::Orientation orientation) : QWidget(parent)
{
   m_restoreActive = false;

   this->setFocusPolicy(Qt::NoFocus);
   m_slider = new ZoomScrollbarSlider(orientation);
   m_slider->setFocusPolicy(Qt::NoFocus);

   ScrollbarButton* first = new ScrollbarButton(ScrollbarButton::FIRST, orientation);
   ScrollbarButton* stepPrevious = new ScrollbarButton(ScrollbarButton::STEP_PREVIOUS, orientation);
   ScrollbarButton* stepNext = new ScrollbarButton(ScrollbarButton::STEP_NEXT, orientation);
   ScrollbarButton* last = new ScrollbarButton(ScrollbarButton::LAST, orientation);
   first->setFocusPolicy(Qt::NoFocus);
   stepPrevious->setFocusPolicy(Qt::NoFocus);
   stepNext->setFocusPolicy(Qt::NoFocus);
   last->setFocusPolicy(Qt::NoFocus);

   int boxLen;
   QBoxLayout* layout;
   if (orientation == Qt::Horizontal)
   {
      layout = new QBoxLayout(QBoxLayout::LeftToRight);
      boxLen = m_slider->minimumHeight();
   }
   else
   {
      layout = new QBoxLayout(QBoxLayout::BottomToTop);
      boxLen = m_slider->minimumWidth();
   }

   layout->setSpacing(0);
   layout->setMargin(0);
   layout->addWidget(first);
   layout->addWidget(stepPrevious);
   layout->addWidget(m_slider,1);
   layout->addWidget(stepNext);
   layout->addWidget(last);

   this->setLayout(layout);

   QSize size(boxLen,boxLen);
   first->setFixedSize(size);
   last->setFixedSize(size);
   stepNext->setFixedSize(size);
   stepPrevious->setFixedSize(size);

   SetScrollbar(0,99,15,20);

   connect( first, SIGNAL( clicked() ), this, SLOT( OnFirst() ) );
   connect( last, SIGNAL( clicked() ), this, SLOT( OnLast() ) );
   connect( stepNext, SIGNAL( clicked() ), this, SLOT( OnStepNext() ) );
   connect( stepPrevious, SIGNAL( clicked() ), this, SLOT( OnStepPrevious() ) );
   connect( m_slider, SIGNAL(ThumbResize()), this, SLOT(OnThumbResize()));
   connect( m_slider, SIGNAL(ThumbSlide()), this, SLOT(OnThumbSlide()));
   connect( m_slider, SIGNAL(ThumbDoubleClick()), this, SLOT(OnThumbDoubleClick()));
}

void ZoomScrollbar::SetScrollbar(ZoomScrollbarData_t min, ZoomScrollbarData_t max, ZoomScrollbarData_t visibleStart, ZoomScrollbarData_t visibleEnd)
{
   //validate min/max . . . min has precedence
   if (max < min)
   {
      max = min;
   }
   m_dataMin = min;
   m_dataMax = max;
   m_dataRange = max - min;

   //validate start/end . . . keep size as priority over positions
   ZoomScrollbarData_t size = visibleEnd - visibleStart;
   if (size <= 0 || m_dataRange == 0)
   {
      AdjustValueForMinMax(visibleStart);
      m_dataStart = visibleStart;
      m_dataEnd = m_dataStart;
   }
   else
   {
      if (size > m_dataRange)
      {
         size = m_dataRange;
      }
      AdjustValueForMinMax(visibleStart);
      visibleEnd = visibleStart + size;
      if (AdjustValueForMinMax(visibleEnd))
      {
         visibleStart = visibleEnd - size;
      }

      m_dataStart = visibleStart;
      m_dataEnd = visibleEnd;
   }

   UpdateLogicalThumb();

   //No notification on programatic change
}

bool ZoomScrollbar::AdjustValueForMinMax(ZoomScrollbarData_t& value)
{
   bool adjusted = false;
   if (value < m_dataMin)
   {
      value = m_dataMin;
      adjusted = true;
   }
   else if (value > m_dataMax)
   {
      value = m_dataMax;
      adjusted = true;
   }

   return adjusted;
}


void ZoomScrollbar::UpdateLogicalThumb()
{
   int lower, upper;
   lower = ScaleDataToLogical(m_dataStart, m_slider->GetMin(),m_slider->GetRange(), m_dataMin, m_dataRange);
   upper = ScaleDataToLogical(m_dataEnd, m_slider->GetMin(),m_slider->GetRange(), m_dataMin, m_dataRange);

   m_slider->SetLowerUpper(lower, upper);
}

void ZoomScrollbar::resizeEvent(QResizeEvent *event)
{
   event;
   //keep slider thumb in sync with new size
   m_slider->Init();
   UpdateLogicalThumb();
}

ZoomScrollbarData_t ZoomScrollbar::GetVisibleStart()
{
   return m_dataStart;
}

ZoomScrollbarData_t ZoomScrollbar::GetVisibleEnd()
{
   return m_dataEnd;
}

bool ZoomScrollbar::FullDisplay()
{
   //determine if we are displaying all the data or not
   //use logical units as the data units can have rounding errors
   return (m_slider->GetLogicalThumbSize() == m_slider->GetRange());
}


void ZoomScrollbar::OnThumbSlide()
{
   //slider thumb moved by slider control interaction
   UpdateDataThumb(true);
   NotifyChanged();
}

void ZoomScrollbar::OnThumbResize()
{
   //slider thumb resized by slider control interaction
   UpdateDataThumb(false);
   NotifyChanged();
}

void ZoomScrollbar::OnThumbDoubleClick()
{
   //restore based on data units
   if (m_slider->GetRange() == m_slider->GetLogicalThumbSize())
   {
      if (m_restoreActive)
      {
         //set scrollbar to restore in case min/max changed and restore position is off
         SetScrollbar(m_dataMin,m_dataMax,m_restoreStart, m_restoreEnd);
         NotifyChanged();
      }
   }
   else
   {
      //switch to 100% . . . remember values to restore to
      m_restoreActive = true;
      m_restoreStart = m_dataStart;
      m_restoreEnd = m_dataEnd;
      m_dataStart = m_dataMin;
      m_dataEnd = m_dataMax;
      UpdateLogicalThumb();
      NotifyChanged();
   }
}

void ZoomScrollbar::NotifyChanged()
{
    emit ValueChanged();
}

void ZoomScrollbar::UpdateDataThumb(bool keepSize)
{
   int lower = m_slider->GetLogicalPosition();
   int upper = lower + m_slider->GetLogicalThumbSize();

   if (keepSize)
   {
      //change start, adjust end as needed trying to keep same size as before
      ZoomScrollbarData_t size = m_dataEnd - m_dataStart;
      m_dataStart = ScaleLogicalToData(lower, m_slider->GetMin(), m_slider->GetRange(), m_dataMin, m_dataRange);
      if (upper == m_slider->GetMax())
      {
         //force to max data value in case of rounding issues
         m_dataEnd = m_dataMax;
      }
      else
      {
         //keep same distance as before if possible
         m_dataEnd = m_dataStart + size;
         if (m_dataEnd > m_dataMax)
         {
            m_dataEnd = m_dataMax;
         }
      }
   }
   else
   {
      //redo both to match underlying slider rounding
      m_dataStart = ScaleLogicalToData(lower, m_slider->GetMin(), m_slider->GetRange(), m_dataMin, m_dataRange);
      m_dataEnd = ScaleLogicalToData(upper, m_slider->GetMin(), m_slider->GetRange(), m_dataMin, m_dataRange);
   }
}

QSize ZoomScrollbar::TEST_GetSliderSize()
{
   return m_slider->size();
}

bool ZoomScrollbar::MoveFirst()
{
   bool res = m_slider->MoveFirst();
   if (res)
   {
      UpdateDataThumb(true);
   }
   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::MoveLast()
{
   bool res = m_slider->MoveLast();
   UpdateDataThumb(true);
   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::MoveNextStep()
{
   bool res = m_slider->MoveNextStep();
   if (res)
   {
      UpdateDataThumb(true);
   }   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::MoveNextPage()
{
   bool res = m_slider->MoveNextPage();
   if (res)
   {
      UpdateDataThumb(true);
   }   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::MovePreviousStep()
{
   bool res = m_slider->MovePreviousStep();
   if (res)
   {
      UpdateDataThumb(true);
   }   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::MovePreviousPage()
{
   bool res = m_slider->MovePreviousPage();
   if (res)
   {
      UpdateDataThumb(true);
   }   //no notification on programatic change
   return res;
}

bool ZoomScrollbar::ExtendStart()
{
   bool res = m_slider->ExtendStart();
   if (res)
   {
      UpdateDataThumb(false);
   }   //no notification on programatic change
   return res;
}

bool ZoomScrollbar::ExtendEnd()
{
   bool res = m_slider->ExtendEnd();
   if (res)
   {
      UpdateDataThumb(false);
   }   //no notification on programatic change
   return res;
}

bool ZoomScrollbar::ShrinkStart()
{
   bool res = m_slider->ShrinkStart();
   if (res)
   {
      UpdateDataThumb(false);
   }   //no notification on programatic change
   return res;
}
bool ZoomScrollbar::ShrinkEnd()
{
   bool res = m_slider->ShrinkEnd();
   if (res)
   {
      UpdateDataThumb(false);
   }   //no notification on programatic change
   return res;
}

void ZoomScrollbar::OnFirst()
{
   if (MoveFirst())
   {
      NotifyChanged();
   }
}
void ZoomScrollbar::OnLast()
{
   if (MoveLast())
   {
      NotifyChanged();
   }
}
void ZoomScrollbar::OnStepNext()
{
   if (MoveNextStep())
   {
      NotifyChanged();
   }
}
void ZoomScrollbar::OnStepPrevious()
{
   if (MovePreviousStep())
   {
      NotifyChanged();
   }
}


}
