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

/*! Adapted from qt-project.org/doc/qt-5/qtwebkitexamples-webkitwidgets-browser-urllineedit-cpp.html
 * This is an edit control that can also act as a progress bar, similar to
 * a web browser's address window */
#include "ProgressLineEdit.h"
#include <QtWidgets>

namespace terbit
{


static const uint32_t FileProgBarMax = 100;
static const uint32_t FileProgBarMin = 0;

ProgressLineEdit::ProgressLineEdit(QWidget *parent) :
   QLineEdit(parent)
{
   m_defaultBaseColor = palette().color(QPalette::Base);
}

QLinearGradient ProgressLineEdit::generateGradient(const QColor &color) const
{
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_defaultBaseColor);
    gradient.setColorAt(0.15, color.lighter(120));
    gradient.setColorAt(0.5, color);//Qt::transparent);
    gradient.setColorAt(0.85, color.lighter(120));
    gradient.setColorAt(1, m_defaultBaseColor);
    return gradient;
}

void ProgressLineEdit::SetProgress(double p)
{
   if(p < FileProgBarMin)
   {
      p = FileProgBarMin;
   }
   else if (p > FileProgBarMax)
   {
      p = FileProgBarMax;
   }
   m_progress = p;
}

void ProgressLineEdit::paintEvent(QPaintEvent *event)
{
   QPalette p = palette();

   QLineEdit::paintEvent(event);

   // note - here we can set different colors based on different things,
   // if we wanted to.
   p.setBrush(QPalette::Base, m_defaultBaseColor);
   setPalette(p);

   QPainter painter(this);
   QStyleOptionFrameV2 panel;
   initStyleOption(&panel);
   QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

   QColor loadingColor = QColor(116, 192, 250);
   loadingColor.setAlpha(150);
   painter.setBrush(generateGradient(loadingColor));
   painter.setPen(Qt::transparent);
   int mid = int((double)backgroundRect.width() / 100.0 * m_progress);
   QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
   painter.drawRect(progressRect);
   //setText(text());
}

#if 0
void ProgressLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;
    if (isReadOnly())
        option->state |= QStyle::State_ReadOnly;
#ifdef QT_KEYPAD_NAVIGATION
    if (hasEditFocus())
        option->state |= QStyle::State_HasEditFocus;
#endif
    option->features = QStyleOptionFrameV2::None;
}
#endif

}
