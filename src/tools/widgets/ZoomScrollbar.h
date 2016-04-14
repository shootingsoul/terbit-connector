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

#include <QEvent>
#include <QScrollBar>
#include <QStyleOptionSlider>

namespace terbit
{


typedef double ZoomScrollbarData_t;

class ZoomScrollbarSlider;


class ZoomScrollbar: public QWidget
{
   Q_OBJECT
public:
   ZoomScrollbar(QWidget* parent, Qt::Orientation orientation = Qt::Horizontal);

   void SetScrollbar(ZoomScrollbarData_t min, ZoomScrollbarData_t max, ZoomScrollbarData_t visibleStart, ZoomScrollbarData_t visibleEnd);

   ZoomScrollbarData_t GetVisibleStart();
   ZoomScrollbarData_t GetVisibleEnd();

   ZoomScrollbarData_t GetMin() { return m_dataMin; }
   ZoomScrollbarData_t GetMax() { return m_dataMax; }
   ZoomScrollbarData_t GetRange() { return m_dataRange; }

   bool FullDisplay();

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

   QSize TEST_GetSliderSize();

signals:
    void ValueChanged();

public slots:

   void OnThumbSlide();
   void OnThumbResize();
   void OnThumbDoubleClick();
   void OnFirst();
   void OnLast();
   void OnStepPrevious();
   void OnStepNext();

protected:
   void resizeEvent(QResizeEvent *event);

private:
   ZoomScrollbar(const ZoomScrollbar& o); //disable copy ctor

   void NotifyChanged();

   bool AdjustValueForMinMax(ZoomScrollbarData_t& value);
   void UpdateLogicalThumb();
   void UpdateDataThumb(bool keepSize);

   ZoomScrollbarSlider* m_slider;

   //data units
   ZoomScrollbarData_t m_dataMin, m_dataMax, m_dataRange, m_dataStart, m_dataEnd, m_restoreStart, m_restoreEnd;
   bool m_restoreActive;

};

}
