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
/*! Adapted from qt-project.org/doc/qt-5/qtwebkitexamples-webkitwidgets-browser-urllineedit-h.html
 * This is an edit control that can also act as a progress bar, similar to
 * a web browser's address window */
#include <QLineEdit>
#include "stdint.h"

QT_BEGIN_NAMESPACE
class QColor;
class QLinearGradient;
QT_END_NAMESPACE

namespace terbit
{

class ProgressLineEdit : public QLineEdit
{
   Q_OBJECT
public:
   explicit ProgressLineEdit(QWidget *parent = 0);
   //void initStyleOption(QStyleOptionFrameV2 *option) const;
   void SetProgress(double p);
   
protected:
   void paintEvent(QPaintEvent *event);

private:
   QLinearGradient generateGradient(const QColor &color) const;
   QColor m_defaultBaseColor;
   uint32_t m_progress;
   
};

}
