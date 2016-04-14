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
#include <cinttypes>

QT_FORWARD_DECLARE_CLASS(QStylePainter)

namespace terbit
{

typedef uint64_t BigScrollbar_t;

class BigScrollbarSlider : public QWidget
{
   Q_OBJECT

public:
   explicit BigScrollbarSlider(Qt::Orientation orientation, QWidget* parent = 0);
   virtual ~BigScrollbarSlider();

   void SetScrollbar(BigScrollbar_t first, BigScrollbar_t range, BigScrollbar_t pos);

   bool SetPosition(BigScrollbar_t pos);
   BigScrollbar_t GetPosition() { return m_position; }

   BigScrollbar_t GetPageStep() { return m_pageStep; }
   void SetPageStep(BigScrollbar_t pageStep);

   BigScrollbar_t GetSingleStep() { return m_singleStep; }
   void SetSingleStep(BigScrollbar_t singleStep);

   BigScrollbar_t GetFirstPostion() { return m_first; }
   BigScrollbar_t GetRange() { return m_range; }

   bool MoveFirst();
   bool MoveLast();
   bool MoveNextStep();
   bool MoveNextPage();
   bool MovePreviousStep();
   bool MovePreviousPage();

Q_SIGNALS:
   void ThumbSlide();

protected:
   virtual void mousePressEvent(QMouseEvent* event);
   virtual void mouseMoveEvent(QMouseEvent* event);
   virtual void mouseReleaseEvent(QMouseEvent* event);
   virtual void paintEvent(QPaintEvent* event);
   virtual void resizeEvent(QResizeEvent *event);

private:

   static const int MIN_THUMB_SIZE;
   static const int THUMB_SIZE_FIXED;

   enum State
   {
      STATE_IDLE,
      STATE_THUMB_MOVING
   };

   void DrawThumb(QStylePainter* painter, const QRect& rect) const;
   bool UpdateThumbRectFromPosition();
   void AdjustPositionForMinMax(BigScrollbar_t& pos);
   bool MoveTo(BigScrollbar_t pos);

   int m_mouseOffset;
   State m_state;
   QRect m_thumbRect;
   BigScrollbar_t m_position, m_pageStep, m_singleStep;
   BigScrollbar_t m_first, m_range;
   Qt::Orientation m_orientation;
};

}
