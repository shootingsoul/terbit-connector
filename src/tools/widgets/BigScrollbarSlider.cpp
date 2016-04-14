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

#include "BigScrollbarSlider.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <tools/Tools.h>

namespace terbit
{

const int BigScrollbarSlider::MIN_THUMB_SIZE = 15;
const int BigScrollbarSlider::THUMB_SIZE_FIXED = 15;


BigScrollbarSlider::BigScrollbarSlider(Qt::Orientation orientation, QWidget* parent) : QWidget(parent),
   m_mouseOffset(0),
   m_state(STATE_IDLE),
   m_position(0), m_pageStep(1), m_singleStep(1),
   m_first(0), m_range(0),
   m_orientation(orientation)
{
   setPalette(QApplication::palette());
   if (m_orientation == Qt::Horizontal)
   {
      this->setMinimumHeight(THUMB_SIZE_FIXED);
      this->setMinimumWidth(MIN_THUMB_SIZE*2); //include breathing room to click on gutter
   }
   else
   {
      this->setMinimumHeight(MIN_THUMB_SIZE*2);//include breathing room to click on gutter
      this->setMinimumWidth(THUMB_SIZE_FIXED);
   }   
   m_thumbRect.setRect(0,0,0,0);
}

BigScrollbarSlider::~BigScrollbarSlider()
{
}

void BigScrollbarSlider::SetScrollbar(BigScrollbar_t first, BigScrollbar_t range, BigScrollbar_t pos)
{
   m_first = first;
   m_range = range;
   AdjustPositionForMinMax(pos);
   m_position = pos;
   if (UpdateThumbRectFromPosition())
   {
      this->update();
   }
}

void BigScrollbarSlider::DrawThumb(QStylePainter* painter, const QRect& rect) const
{

   QStyleOptionButton opt;
   opt.state = QStyle::State_Active | QStyle::State_Enabled;
   opt.rect = rect;

   if (m_state == STATE_THUMB_MOVING)
   {
      opt.state |= QStyle::State_Sunken;
   }

   style()->drawControl(QStyle::CE_PushButton, &opt,painter);

   QPen pen(palette().color(QPalette::ButtonText), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
   painter->setPen(pen);

   if (m_orientation == Qt::Horizontal)
   {
      int x,y,y2;
      x = rect.center().x();
      y = 5;
      y2 = rect.height() - 5;
      painter->drawLine(x,y,x,y2);

      x -= 3;
      painter->drawLine(x,y,x,y2);
      x += 6;
      painter->drawLine(x,y,x,y2);
   }
   else
   {
      int x,x2,y;
      y = rect.center().y();
      x = 5;
      x2 = rect.width() - 5;
      painter->drawLine(x,y,x2,y);

      y -= 3;
      painter->drawLine(x,y,x2,y);

      y += 6;
      painter->drawLine(x,y,x2,y);
   }
}


bool BigScrollbarSlider::SetPosition(BigScrollbar_t pos)
{   
   //move with validation
   if (m_range > 0)
   {
      AdjustPositionForMinMax(pos);

      return MoveTo(pos);
   }
   else
   {
      return false;
   }
}

void BigScrollbarSlider::SetPageStep(BigScrollbar_t pageStep)
{
   m_pageStep = pageStep;
}

void BigScrollbarSlider::SetSingleStep(BigScrollbar_t singleStep)
{
   m_singleStep = singleStep;
}

bool BigScrollbarSlider::MoveTo(BigScrollbar_t pos)
{
   //assumes pos already validated . . .
   bool changeMade = false;
   if (pos != m_position)
   {
      m_position = pos;
      changeMade = true;
      if (UpdateThumbRectFromPosition())
      {
         this->update();
      }
   }
   return changeMade;
}

bool BigScrollbarSlider::MoveFirst()
{
   if (m_range > 0)
   {
      return MoveTo(m_first);
   }
   else
   {
      return false;
   }
}
bool BigScrollbarSlider::MoveLast()
{
   if (m_range > 0)
   {
      return MoveTo(m_first + m_range - 1);
   }
   else
   {
      return false;
   }
}
bool BigScrollbarSlider::MoveNextStep()
{
   if (m_singleStep < m_range && m_range + m_first - m_singleStep > m_position)
   {
      return MoveTo(m_position+m_singleStep);
   }
   else if (m_range != 0)
   {
      return MoveTo(m_first + m_range - 1);
   }
   else
   {
      return false;
   }
}
bool BigScrollbarSlider::MoveNextPage()
{
   if (m_pageStep < m_range && m_range + m_first - m_pageStep > m_position)
   {
      return MoveTo(m_position+m_pageStep);
   }
   else if (m_range != 0)
   {
      return MoveTo(m_first + m_range - 1);
   }
   else
   {
      return false;
   }
}
bool BigScrollbarSlider::MovePreviousStep()
{
   if (m_singleStep <= m_position - m_first)
   {
      return MoveTo(m_position-m_singleStep);
   }
   else if (m_range != 0)
   {
      return MoveTo(m_first);
   }
   else
   {
      return false;
   }
}
bool BigScrollbarSlider::MovePreviousPage()
{
   if (m_pageStep <= m_position - m_first)
   {
      return MoveTo(m_position-m_pageStep);
   }
   else if (m_range != 0)
   {
      return MoveTo(m_first);
   }
   else
   {
      return false;
   }
}

bool BigScrollbarSlider::UpdateThumbRectFromPosition()
{
   bool changeMade = false;

   int gutterSize = (m_orientation == Qt::Horizontal ? width() : height());
   int thumbSize;
   int guiPos;
   if (m_range <= 1)
   {
      thumbSize = gutterSize;
      guiPos = 0;
   }
   else
   {
       //limit max size of thumb within the slider area
      //determine thumb size based on page step . . . validate page step within range
      uint64_t dataThumb = m_pageStep;
      if (m_pageStep > m_range)
      {
         dataThumb = m_range;
      }
      thumbSize = ScaleDataToLogical(dataThumb,0,(int)(gutterSize *0.80),(BigScrollbar_t)0,m_range);
      if (thumbSize < MIN_THUMB_SIZE)
      {
         thumbSize = MIN_THUMB_SIZE;
      }
      int guiRange = gutterSize - thumbSize+1;

      //force to end in case of truncation and at end of range
      if (m_position == m_range - 1)
      {
         guiPos = guiRange-1;
      }
      else
      {
         guiPos = ScaleDataToLogical(m_position,0,guiRange,m_first,m_range);
      }
   }

   int x,w,y,h;
   if (m_orientation == Qt::Horizontal)
   {
      x = guiPos;
      y = 0;
      w = thumbSize;
      h = this->height();
   }
   else
   {
      x = 0;
      y = guiPos;
      w = this->width();
      h = thumbSize;
   }

   if (m_thumbRect.x() != x || m_thumbRect.width() != w || m_thumbRect.y() != y || m_thumbRect.height() != h)
   {
      m_thumbRect.setRect(x,y,w,h);
      changeMade = true;
   }
   return changeMade;
}

void BigScrollbarSlider::mousePressEvent(QMouseEvent* event)
{
   if (m_range == 0 || ((event->buttons() & Qt::LeftButton) != Qt::LeftButton))
   {
      event->ignore();
      return;
   }

   const QPoint pos = event->pos();

   if (m_thumbRect.contains(pos))
   {
      //movement based on top left of thumb rect
      if (m_orientation == Qt::Horizontal)
      {
         m_mouseOffset = pos.x() - m_thumbRect.x();
      }
      else
      {
         m_mouseOffset = pos.y() - m_thumbRect.y();
      }
      m_state = STATE_THUMB_MOVING;
   }
   else
   {
      //clicked in the gutter, so page next/prev accordingly
      if (((m_orientation == Qt::Horizontal) && (pos.x() < m_thumbRect.x())) || ((m_orientation != Qt::Horizontal) && pos.y() < m_thumbRect.y()))
      {
         if (MovePreviousPage())
         {
            emit ThumbSlide();
         }
      }
      else
      {
         if (MoveNextPage())
         {
            emit ThumbSlide();
         }
      }
      //ensure we're still in IDLE
      m_state = STATE_IDLE;
   }
   event->accept();
}

void BigScrollbarSlider::AdjustPositionForMinMax(BigScrollbar_t& pos)
{
   if (m_range == 0)
   {
      pos = 0;
   }
   else if (pos < m_first)
   {
      pos = m_first;
   }
   else if (pos >= m_first + m_range)
   {
      pos = m_first + m_range - 1;
   }
}

void BigScrollbarSlider::mouseMoveEvent(QMouseEvent* event)
{
   if (m_state == STATE_IDLE)
   {
      event->ignore();
      return;
   }

   const QPoint pos = event->pos();
   int guiPos, guiRange, guiStart, thumbSize;

   switch (m_state)
   {
   case STATE_THUMB_MOVING:
      {
         //calculate to start of the thumb
         //don't include thumb in gui range
         if (m_orientation == Qt::Horizontal)
         {
            thumbSize = m_thumbRect.width();
            guiStart = 0;
            guiPos = pos.x() - m_mouseOffset;
            guiRange = this->width() - thumbSize + 1;

            if (guiPos < 0)
            {
               guiPos = 0;
            }
            else if (guiPos >= guiRange)
            {
               guiPos = guiRange-1;
            }

            m_thumbRect.setRect(guiPos,0,thumbSize,m_thumbRect.height());
            update();
         }
         else
         {
            thumbSize = m_thumbRect.height();
            guiStart = 0;
            guiPos = pos.y() - m_mouseOffset;
            guiRange = this->height() - thumbSize + 1;

            if (guiPos < 0)
            {
               guiPos = 0;
            }
            else if (guiPos >= guiRange)
            {
               guiPos = guiRange-1;
            }

            m_thumbRect.setRect(0,guiPos,m_thumbRect.width(),thumbSize);
            update();
         }

         BigScrollbar_t newPos;
         //force to end if at last gui pos . . . scaling may not allow it to be directly reached
         if (guiPos == guiRange-1)
         {
            newPos = m_range-1;
         }
         else
         {
            newPos = ScaleLogicalToData(guiPos,guiStart,guiRange,m_first,m_range);
         }
         if (newPos != m_position)
         {
            m_position = newPos;
            emit ThumbSlide();
         }

      }
      break;
   };   

   event->accept();
}


void BigScrollbarSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (m_state == STATE_THUMB_MOVING)
    {
      m_state = STATE_IDLE;
      update();
    }
}

void BigScrollbarSlider::paintEvent(QPaintEvent* event)
{
   Q_UNUSED(event);
   QStylePainter painter(this);

   //gutter background
   QRect rect = this->rect();
   QColor color;
   //color= QApplication::palette().color(QPalette::Window); //system background color
   color = palette().color(QPalette::AlternateBase);
   painter.setBrush(color);
   painter.setPen(Qt::NoPen);
   rect.adjust(2,2,-2,-2);
   painter.drawRect(rect);

   color = palette().color(QPalette::Dark);
   painter.setBrush(Qt::NoBrush);
   painter.setPen(color);
   rect.adjust(0,0,-1,-1);
   painter.drawRect(rect);

   //draw thumb
   DrawThumb(&painter, m_thumbRect);
}

void BigScrollbarSlider::resizeEvent(QResizeEvent *event)
{
   UpdateThumbRectFromPosition();
   update();
}

}
