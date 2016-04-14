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

#include "BigScrollbar.h"
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

BigScrollbar::BigScrollbar(QWidget* parent, Qt::Orientation orientation) : QWidget(parent)
{
   this->setFocusPolicy(Qt::NoFocus);
   m_slider = new BigScrollbarSlider(orientation);
   m_slider->setFocusPolicy(Qt::NoFocus);

   //graphics are reversed from zoom scrollbar . . . .this is top to bottom
   ScrollbarButton* first = new ScrollbarButton(ScrollbarButton::LAST, orientation);
   ScrollbarButton* stepPrevious = new ScrollbarButton(ScrollbarButton::STEP_NEXT, orientation);
   ScrollbarButton* stepNext = new ScrollbarButton(ScrollbarButton::STEP_PREVIOUS, orientation);
   ScrollbarButton* last = new ScrollbarButton(ScrollbarButton::FIRST, orientation);
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
      layout = new QBoxLayout(QBoxLayout::TopToBottom);
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

   connect( first, SIGNAL( clicked() ), this, SLOT( OnFirst() ) );
   connect( last, SIGNAL( clicked() ), this, SLOT( OnLast() ) );
   connect( stepNext, SIGNAL( clicked() ), this, SLOT( OnStepNext() ) );
   connect( stepPrevious, SIGNAL( clicked() ), this, SLOT( OnStepPrevious() ) );
   connect( m_slider, SIGNAL(ThumbSlide()), this, SLOT(OnThumbSlide()));
}

void BigScrollbar::SetScrollbar(BigScrollbar_t first, BigScrollbar_t range, BigScrollbar_t pos)
{
   m_slider->SetScrollbar(first,range,pos);
}


void BigScrollbar::OnThumbSlide()
{
   //slider thumb moved by slider control interaction
   NotifyChanged();
}

void BigScrollbar::NotifyChanged()
{
    emit PositionChanged();
}

bool BigScrollbar::MoveFirst()
{
   bool res = m_slider->MoveFirst();
   //no notification on programatic change
   return res;
}
bool BigScrollbar::MoveLast()
{
   bool res = m_slider->MoveLast();
   //no notification on programatic change
   return res;
}
bool BigScrollbar::MoveNextStep()
{
   bool res = m_slider->MoveNextStep();
   return res;
}
bool BigScrollbar::MoveNextPage()
{
   bool res = m_slider->MoveNextPage();
   return res;
}
bool BigScrollbar::MovePreviousStep()
{
   bool res = m_slider->MovePreviousStep();
   return res;
}
bool BigScrollbar::MovePreviousPage()
{
   bool res = m_slider->MovePreviousPage();
   return res;
}
void BigScrollbar::OnFirst()
{
   if (MoveFirst())
   {
      NotifyChanged();
   }
}
void BigScrollbar::OnLast()
{
   if (MoveLast())
   {
      NotifyChanged();
   }
}
void BigScrollbar::OnStepNext()
{
   if (MoveNextStep())
   {
      NotifyChanged();
   }
}
void BigScrollbar::OnStepPrevious()
{
   if (MovePreviousStep())
   {
      NotifyChanged();
   }
}


}

