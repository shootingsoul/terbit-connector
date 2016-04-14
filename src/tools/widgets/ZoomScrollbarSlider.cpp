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
#include "ZoomScrollbarSlider.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <tools/Tools.h>

namespace terbit
{

const int ZoomScrollbarSlider::HANDLE_SIZE = 11;
const int ZoomScrollbarSlider::HANDLE_SIZE_LONG = 15;

ZoomScrollbarSlider::ZoomScrollbarSlider(Qt::Orientation orientation, QWidget* parent) : QWidget(parent),
   m_mouseOffset(0),
   m_state(STATE_IDLE),
   m_min(0),
   m_max(0),
   m_range(0),
   m_position(0),
   m_drawHandleAFirst(true),
   m_orientation(orientation)
{
   setPalette(QApplication::palette());
   if (m_orientation == Qt::Horizontal)
   {
      this->setMinimumHeight(HANDLE_SIZE_LONG);
      this->setMinimumWidth(HANDLE_SIZE+1); //two handles displayed, allow for overlap but not matching
      m_handleARect.setRect(0,0,HANDLE_SIZE, HANDLE_SIZE_LONG);
      m_handleBRect.setRect(0,0,HANDLE_SIZE, HANDLE_SIZE_LONG);
   }
   else
   {
      this->setMinimumHeight(HANDLE_SIZE+1); //two handles displayed, allow for overlap but not matching
      this->setMinimumWidth(HANDLE_SIZE_LONG);
      m_handleARect.setRect(0,0,HANDLE_SIZE_LONG, HANDLE_SIZE);
      m_handleBRect.setRect(0,0,HANDLE_SIZE_LONG, HANDLE_SIZE);
   }   
   m_thumbRect.setRect(0,0,0,0);
   Init();
}

ZoomScrollbarSlider::~ZoomScrollbarSlider()
{
}

void ZoomScrollbarSlider::SetupSpanPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const
{
   QColor highlight = palette().color(QPalette::Highlight);
   QLinearGradient gradient(x1, y1, x2, y2);
   gradient.setColorAt(0, highlight.dark(120));
   gradient.setColorAt(1, highlight.light(108));
   painter->setBrush(gradient);

   if (orientation == Qt::Horizontal)
   {
      painter->setPen(QPen(highlight.dark(130), 0));
   }
   else
   {
      painter->setPen(QPen(highlight.dark(150), 0));
   }
}

void ZoomScrollbarSlider::DrawSpan(QStylePainter* painter, const QRect& groove) const
{
   // pen & brush
   painter->setPen(QPen(palette().color(QPalette::AlternateBase), 0));
   if (m_orientation == Qt::Horizontal)
   {
      SetupSpanPainter(painter, m_orientation, groove.center().x(), groove.top(), groove.center().x(), groove.y()+groove.height());
   }
   else
   {
      SetupSpanPainter(painter, m_orientation, groove.left(), groove.center().y(), groove.x()+groove.width(), groove.center().y());
   }

   // draw groove
   painter->drawRect(groove);
}

void ZoomScrollbarSlider::DrawHandle(QStylePainter* painter, const QRect& rect) const
{
   QStyleOptionButton opt;
   opt.state = QStyle::State_Active | QStyle::State_Enabled;
   opt.rect = rect;

   if (m_state == STATE_HANDLE_A_MOVING || m_state == STATE_HANDLE_B_MOVING)
   {
      opt.state |= QStyle::State_Sunken;
   }

   style()->drawControl(QStyle::CE_PushButton, &opt,painter);

   QPen pen(palette().color(QPalette::ButtonText), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
   painter->setPen(pen);

   if (m_orientation == Qt::Horizontal)
   {
      int x,y,y2;
      x = rect.center().x()-1;
      y = 5;
      y2 = rect.height() - 5;
      painter->drawLine(x,y,x,y2);

      x += 2;
      painter->drawLine(x,y,x,y2);
   }
   else
   {
      int x,x2,y;
      y = rect.center().y()-1;
      x = 5;
      x2 = rect.width() - 5;
      painter->drawLine(x,y,x2,y);

      y += 2;
      painter->drawLine(x,y,x2,y);
   }
}

void ZoomScrollbarSlider::DrawThumb(QStylePainter* painter, const QRect& rect) const
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

void ZoomScrollbarSlider::Init()
{
   if (m_orientation == Qt::Horizontal)
   {
      m_min = HANDLE_SIZE/2;
      m_max = this->width()-HANDLE_SIZE/2;
   }
   else
   {      
      m_min = HANDLE_SIZE/2;
      m_max = this->height()-HANDLE_SIZE/2;
   }

   m_range = m_max-m_min;
}

bool ZoomScrollbarSlider::SetLowerUpper(int lower, int upper)
{
   //public function . . . logical co-ordinates
   bool changeMade;
   //try to keep the size, move lower/upper if needed
   int size = upper - lower;
   if (size <= 0)
   {
      size = 1;
   }
   AdjustPositionForMinMax(lower);
   if ((lower+size) > m_max)
   {
      lower = m_max-size;
      if (lower < m_min)
      {
         lower = m_min;
      }
   }
   upper = lower + size;
   AdjustPositionForMinMax(upper);

   if (m_orientation == Qt::Horizontal)
   {
      SetHandleAPosition(lower);
      SetHandleBPosition(upper);
   }
   else
   {
      //veritcal reverse logic (lower will have higher screen co-ordinates)
      lower = ReversePosition(lower, m_min, m_range);
      upper = ReversePosition(upper, m_min, m_range);
      SetHandleAPosition(upper);
      SetHandleBPosition(lower);
   }

   changeMade = UpdateThumbRect();
   if (changeMade)
   {      
      this->update();
   }
   //no emit for programatic change, let calling function emit if it's an interactive user event
   return changeMade;
}

bool ZoomScrollbarSlider::MoveTo(int lower)
{
   //SetUpperLower tries to keep size the same and adjusts lower/upper as needed
   //so we can use that for moving too
   return SetLowerUpper(lower,lower+GetLogicalThumbSize());
}

bool ZoomScrollbarSlider::MoveFirst()
{
   return MoveTo(m_min);
}
bool ZoomScrollbarSlider::MoveLast()
{
   return MoveTo(m_max-GetLogicalThumbSize());
}
bool ZoomScrollbarSlider::MoveNextStep()
{
   return MoveTo(m_position+1);
}
bool ZoomScrollbarSlider::MoveNextPage()
{
   return MoveTo(m_position+GetLogicalThumbSize());
}
bool ZoomScrollbarSlider::MovePreviousStep()
{
   return MoveTo(m_position-1);
}
bool ZoomScrollbarSlider::MovePreviousPage()
{
   return MoveTo(m_position-GetLogicalThumbSize());
}

bool ZoomScrollbarSlider::ExtendStart()
{
   bool res;
   if (m_position != m_min)
   {
      res = SetLowerUpper(m_position-1,m_position+GetLogicalThumbSize());
   }
   else
   {
      res = false;
   }
   return res;
}

bool ZoomScrollbarSlider::ExtendEnd()
{
   bool res;
   if (m_position+GetLogicalThumbSize() != m_max)
   {
      res = SetLowerUpper(m_position,m_position+GetLogicalThumbSize()+1);
   }
   else
   {
      res = false;
   }
   return res;
}
bool ZoomScrollbarSlider::ShrinkStart()
{
   bool res;
   if (GetLogicalThumbSize() > 1)
   {
      res = SetLowerUpper(m_position+1,m_position+GetLogicalThumbSize());
   }
   else
   {
      res = false;
   }
   return res;
}

bool ZoomScrollbarSlider::ShrinkEnd()
{
   bool res;
   if (GetLogicalThumbSize() > 1)
   {
      res = SetLowerUpper(m_position,m_position+GetLogicalThumbSize()-1);
   }
   else
   {
      res = false;
   }
   return res;
}

bool ZoomScrollbarSlider::UpdateThumbRect()
{
   bool changeMade = false;

   QRect* lower = &m_handleARect;
   QRect* upper = &m_handleBRect;
   int x,w,y,h;
   if (m_orientation == Qt::Horizontal)
   {      
      if (lower->x() > upper->x())
      {
         lower = &m_handleBRect;
         upper = &m_handleARect;
      }
      x = lower->x()+HANDLE_SIZE/2;
      y = 0;
      w = upper->x()-lower->x();
      h = this->height();
   }
   else
   {
      if (lower->y() > upper->y())
      {
         lower = &m_handleBRect;
         upper = &m_handleARect;
      }
      x = 0;
      y = lower->y()+HANDLE_SIZE/2;
      w = this->width();
      h = upper->y()-lower->y();
   }

   if (m_thumbRect.x() != x || m_thumbRect.width() != w || m_thumbRect.y() != y || m_thumbRect.height() != h)
   {
      m_thumbRect.setRect(x,y,w,h);
      if (m_orientation == Qt::Horizontal)
      {
         m_position = m_thumbRect.x();
      }
      else
      {
         //use reverse logic for vertical
         m_position = ReversePosition(m_thumbRect.y()+m_thumbRect.height(), m_min, m_range);
      }
      changeMade = true;
   }
   return changeMade;
}

void ZoomScrollbarSlider::mouseDoubleClickEvent(QMouseEvent* event)
{
   if (m_thumbRect.contains(event->pos()))
   {
      emit ThumbDoubleClick();
      event->accept();
   }
   else
   {
      event->ignore();
   }
}

void ZoomScrollbarSlider::mousePressEvent(QMouseEvent* event)
{
   if (m_min == m_max || ((event->buttons() & Qt::LeftButton) != Qt::LeftButton))
   {
      event->ignore();
      return;
   }

   const QPoint pos = event->pos();

   if (m_handleBRect.contains(pos))
   {
      if (m_orientation == Qt::Horizontal)
      {
         m_mouseOffset = pos.x() - (m_handleBRect.x()+HANDLE_SIZE/2);
      }
      else
      {
         m_mouseOffset = pos.y() - (m_handleBRect.y()+HANDLE_SIZE/2);
      }
      m_state = STATE_HANDLE_B_MOVING;
      m_drawHandleAFirst = true;
   }
   else if (m_handleARect.contains(pos))
   {
      if (m_orientation == Qt::Horizontal)
      {
         m_mouseOffset = pos.x() - (m_handleARect.x()+HANDLE_SIZE/2);
      }
      else
      {
         m_mouseOffset = pos.y() - (m_handleARect.y()+HANDLE_SIZE/2);
      }
      m_state = STATE_HANDLE_A_MOVING;
      m_drawHandleAFirst = false;
   }
   else if (m_thumbRect.contains(pos))
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
      if (((m_orientation == Qt::Horizontal) && (pos.x() < m_thumbRect.x())) || ((m_orientation != Qt::Horizontal) && (pos.y() > m_thumbRect.y()+m_thumbRect.height())))
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

bool ZoomScrollbarSlider::AdjustPositionForMinMax(int& pos)
{
   bool adjusted = false;
   if (pos < m_min)
   {
      pos = m_min;
      adjusted = true;
   }
   else if (pos > m_max)
   {
      pos = m_max;
      adjusted = true;
   }
   return adjusted;
}

void ZoomScrollbarSlider::mouseMoveEvent(QMouseEvent* event)
{
   if (m_state == STATE_IDLE)
   {
      event->ignore();
      return;
   }

   bool changeMade = false;
   const QPoint pos = event->pos();
   int newPosition;

   switch (m_state)
   {
   case STATE_HANDLE_A_MOVING:
      if (m_orientation == Qt::Horizontal)
      {
         newPosition = pos.x() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);
         if (newPosition != m_handleBRect.x()+HANDLE_SIZE/2)
         {
            SetHandleAPosition(newPosition);
         }
      }
      else
      {
         newPosition = pos.y() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);
         if (newPosition != m_handleBRect.y()+HANDLE_SIZE/2)
         {
            SetHandleAPosition(newPosition);
         }
      }
      changeMade = UpdateThumbRect();
      break;
   case STATE_HANDLE_B_MOVING:
      if (m_orientation == Qt::Horizontal)
      {
         newPosition = pos.x() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);
         if (newPosition != m_handleARect.x()+HANDLE_SIZE/2)
         {
            SetHandleBPosition(newPosition);
         }
      }
      else
      {
         newPosition = pos.y() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);
         if (newPosition != m_handleARect.y()+HANDLE_SIZE/2)
         {
            SetHandleBPosition(newPosition);
         }
      }
      changeMade = UpdateThumbRect();
      break;
   case STATE_THUMB_MOVING:
      if (m_orientation == Qt::Horizontal)
      {
         newPosition = pos.x() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);

         int otherNew = m_thumbRect.width() + newPosition;
         if (AdjustPositionForMinMax(otherNew))
         {
            newPosition = otherNew - m_thumbRect.width();
         }
         SetHandleAPosition(newPosition);
         SetHandleBPosition(otherNew);
      }
      else
      {
         newPosition = pos.y() - m_mouseOffset;
         AdjustPositionForMinMax(newPosition);

         int otherNew = m_thumbRect.height() + newPosition;
         if (AdjustPositionForMinMax(otherNew))
         {
            newPosition = otherNew - m_thumbRect.height();
         }
         SetHandleAPosition(newPosition);
         SetHandleBPosition(otherNew);
      }
      changeMade = UpdateThumbRect();
      if (changeMade)
      {         
         emit ThumbSlide();
      }
      break;
   };   

   if (changeMade)
   {
      update();
   }
   event->accept();
}

void ZoomScrollbarSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (m_state == STATE_HANDLE_A_MOVING || m_state == STATE_HANDLE_B_MOVING)
    {
      emit ThumbResize();
      m_state = STATE_IDLE;
      update();
    }
    else if (m_state == STATE_THUMB_MOVING)
    {
      m_state = STATE_IDLE;
      update();
    }
}

void ZoomScrollbarSlider::SetHandleBPosition(int newPos)
{
   newPos -= HANDLE_SIZE/2;
   const QRect& r = m_handleBRect;
   if (m_orientation==Qt::Horizontal)
   {
      if (r.x() != newPos)
      {
         m_handleBRect.setRect(newPos,r.y(),r.width(),r.height());
      }
   }
   else
   {
      if (r.y() != newPos)
      {
         m_handleBRect.setRect(r.x(),newPos,r.width(),r.height());
      }
   }
}

void ZoomScrollbarSlider::SetHandleAPosition(int newPos)
{
   newPos -= HANDLE_SIZE/2;
   const QRect& r = m_handleARect;
   if (m_orientation==Qt::Horizontal)
   {
      if (r.x() != newPos)
      {
         m_handleARect.setRect(newPos,r.y(),r.width(),r.height());
      }
   }
   else
   {
      if (r.y() != newPos)
      {
         m_handleARect.setRect(r.x(),newPos,r.width(),r.height());
      }
   }
}

void ZoomScrollbarSlider::paintEvent(QPaintEvent* event)
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

   // span
   //only draw span if moving a handle for ZoomScrollbar
   if (m_state == STATE_HANDLE_A_MOVING || m_state == STATE_HANDLE_B_MOVING)
   {
      int minv, maxv;
      if (m_orientation==Qt::Horizontal)
      {
         minv = m_thumbRect.x();
         maxv = m_thumbRect.x()+m_thumbRect.width();
      }
      else
      {
         minv = m_thumbRect.y();
         maxv = m_thumbRect.y()+m_thumbRect.height();
      }
      const QPoint c = m_thumbRect.center();
      QRect spanRect;
      if (m_orientation == Qt::Horizontal)
      {
         spanRect = QRect(QPoint(minv, c.y() - 2), QPoint(maxv, c.y() + 1));
      }
      else
      {
         spanRect = QRect(QPoint(c.x() - 2, minv), QPoint(c.x() + 1, maxv));
      }
      DrawSpan(&painter, spanRect);
   }
   else
   {
      //draw thumb
      DrawThumb(&painter, m_thumbRect);
   }

   // handles . . . use draw indicate for overlap situations . . . determine who's on top
   if (m_drawHandleAFirst)
   {
      DrawHandle(&painter, m_handleARect);
      DrawHandle(&painter, m_handleBRect);
   }
   else
   {
      DrawHandle(&painter, m_handleBRect);
      DrawHandle(&painter, m_handleARect);
   }
}

}
