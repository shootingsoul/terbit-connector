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
#include "BigScrollbarSlider.h"

namespace terbit
{

class BigScrollbarSlider;


class BigScrollbar: public QWidget
{
   Q_OBJECT
public:
   BigScrollbar(QWidget* parent, Qt::Orientation orientation = Qt::Vertical);

   void SetScrollbar(BigScrollbar_t first, BigScrollbar_t range, BigScrollbar_t pos);

   BigScrollbar_t GetPosition() { return m_slider->GetPosition(); }
   void SetPosition(BigScrollbar_t pos) { m_slider->SetPosition(pos); }

   BigScrollbar_t GetPageStep() { return m_slider->GetPageStep(); }
   void SetPageStep(BigScrollbar_t pageStep) { m_slider->SetPageStep(pageStep); }

   BigScrollbar_t GetSingleStep() { return m_slider->GetSingleStep(); }
   void SetSingleStep(BigScrollbar_t singleStep) { m_slider->SetSingleStep(singleStep); }

   bool MoveFirst();
   bool MoveLast();
   bool MoveNextStep();
   bool MoveNextPage();
   bool MovePreviousStep();
   bool MovePreviousPage();

signals:
    void PositionChanged();

public slots:

   void OnThumbSlide();
   void OnFirst();
   void OnLast();
   void OnStepPrevious();
   void OnStepNext();

private:
   BigScrollbar(const BigScrollbar& o); //disable copy ctor

   void NotifyChanged();
   BigScrollbarSlider* m_slider;
};

}
