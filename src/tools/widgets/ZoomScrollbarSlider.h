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

#include <QStyle>
#include <QObject>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QStylePainter)

namespace terbit
{

class ZoomScrollbarSlider : public QWidget
{
   Q_OBJECT

public:
   explicit ZoomScrollbarSlider(Qt::Orientation orientation, QWidget* parent = 0);
   virtual ~ZoomScrollbarSlider();

   void Init();
   bool SetLowerUpper(int lower, int upper);

   int GetMin() { return m_min; }
   int GetMax() { return m_max; }
   int GetRange() { return m_range; }

   int GetLogicalPosition() { return m_position; }
   int GetLogicalThumbSize() { return (m_orientation == Qt::Horizontal ? m_thumbRect.width() : m_thumbRect.height()); }

   bool MoveFirst();
   bool MoveLast();
   bool MoveNextStep();
   bool MoveNextPage();
   bool MovePreviousStep();
   bool MovePreviousPage();
   bool ExtendStart();
   bool ExtendEnd();
   bool ShrinkStart();
   bool ShrinkEnd();

Q_SIGNALS:
   void ThumbResize();
   void ThumbSlide();
   void ThumbDoubleClick();

protected:
   virtual void mousePressEvent(QMouseEvent* event);
   virtual void mouseMoveEvent(QMouseEvent* event);
   virtual void mouseDoubleClickEvent(QMouseEvent* event);
   virtual void mouseReleaseEvent(QMouseEvent* event);
   virtual void paintEvent(QPaintEvent* event);

private:

   static const int HANDLE_SIZE;
   static const int HANDLE_SIZE_LONG;

   enum State
   {
      STATE_IDLE,
      STATE_HANDLE_A_MOVING,
      STATE_HANDLE_B_MOVING,
      STATE_THUMB_MOVING
   };

   void DrawHandle(QStylePainter* painter, const QRect& rect) const;
   void DrawThumb(QStylePainter* painter, const QRect& rect) const;
   void SetupSpanPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const;
   void DrawSpan(QStylePainter* painter, const QRect& rect) const;
   void SetHandleBPosition(int newPos);
   void SetHandleAPosition(int newPos);
   bool UpdateThumbRect();
   bool AdjustPositionForMinMax(int& pos);
   bool MoveTo(int lower);

   int m_mouseOffset;
   State m_state;
   QRect m_handleARect, m_handleBRect, m_thumbRect;
   int m_min, m_max, m_range, m_position;
   bool m_drawHandleAFirst;
   Qt::Orientation m_orientation;
};

}
