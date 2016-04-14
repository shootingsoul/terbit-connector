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

#include "ScrollbarButton.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QApplication>

namespace terbit
{

ScrollbarButton::ScrollbarButton(ButtonType type,Qt::Orientation orientation) : QPushButton(), m_type(type),m_orientation(orientation)
{
   setPalette(QApplication::palette());
   if (type == STEP_NEXT || type == STEP_PREVIOUS)
   {
      this->setAutoRepeat(true);
   }
}

void ScrollbarButton::paintEvent(QPaintEvent* event)
{
   QPushButton::paintEvent(event);


   QBrush brush = palette().buttonText();


   QPainter painter(this);
   QPen pen(palette().color(QPalette::ButtonText), 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

   QPainterPath path;
   qreal x1, y1,x2,y2,x3,y3, border;
   border = 5.0;

   if (m_type == FIRST || m_type == STEP_PREVIOUS)
   {
      if (m_orientation == Qt::Horizontal)
      {
         x1 = border;
         y1 = this->height()/2.0;
         x2 = this->width()-border;
         y2 = border;
         x3 = x2;
         y3 = this->height()-border;
      }
      else
      {
         x1 = this->width()/2.0;
         y1 = this->height()-border;
         x2 = this->width()-border;
         y2 = border;
         x3 = border;
         y3 = y2;
      }
   }
   else
   {
      if (m_orientation == Qt::Horizontal)
      {
         x1 = this->width()-border;
         y1 = this->height()/2.0;
         x2 = border;
         y2 = border;
         x3 = x2;
         y3 = this->height()-border;
      }
      else
      {
         x1 = this->width()/2.0;
         y1 = border;
         x2 = this->width()-border;
         y2 = this->height()-border;
         x3 = border;
         y3 = y2;
      }
   }

   path.moveTo(x1,y1);
   path.lineTo(x2,y2);
   path.lineTo(x3,y3);
   path.lineTo(x1,y1);

   painter.setPen(Qt::NoPen);
   painter.fillPath(path,brush);

   if (m_type == FIRST || m_type == LAST)
   {
      QLineF line;
      if (m_orientation == Qt::Horizontal)
      {
         line.setLine(x1,y2,x1,y3);
      }
      else
      {
         line.setLine(x2,y1,x3,y1);
      }
      painter.setPen(pen);
      painter.drawLine(line);
   }
}

}
