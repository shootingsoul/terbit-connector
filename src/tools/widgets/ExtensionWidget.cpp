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
#include "ExtensionWidget.h"
#include <QPainter>
#include <QPoint>
#include <QFontMetrics>

namespace terbit
{
static const bool extExpanded = true;
static const bool extCollapsed = false;

ExtensionWidget::ExtensionWidget(QWidget *parent, QString name, QWidget* ext) :
   QWidget(parent), m_ExtensionWidget(ext)
{
   // create the special button
   m_moreButton = new ExtensionButton(this);
   m_moreButton->SetText(name);
   m_moreButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
   m_moreButton->setCheckable(true);
   m_moreButton->setChecked(extCollapsed);
   connect(m_moreButton, SIGNAL(clicked(bool)), this, SLOT(extend(bool)));

   // Create a layout with button at top, and widget below that
   m_layout = new QVBoxLayout;
   m_layout->setMargin(0);
   m_layout->addWidget(m_moreButton);
   m_layout->addWidget(m_ExtensionWidget);
   m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

   // Remove spacing between button and widget space; widget will probably
   // have margin already.
   m_layout->setSpacing(0);

   setLayout(m_layout);

   setWindowTitle(name);
   if(m_ExtensionWidget)
   {
      m_ExtensionWidget->hide();
   }
}


void ExtensionWidget::extend(bool extend)
{
   m_ExtensionWidget->setVisible(extend == extExpanded);
}

bool ExtensionWidget::GetCollapsed() const
{
   if(m_moreButton)
   {
      return m_moreButton->isChecked() == extCollapsed;
   }
   else
   {
      return false;
   }
}

// --------------------------- Extension Button ------------------------


ExtensionButton::ExtensionButton(QWidget *parent) : QPushButton(parent)
{
   QImage *tmp = new QImage(":/img/stylesheet-branch-closed.png");

   m_collapsedImage = tmp->scaled(QSize(7,6), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
   tmp = new QImage(":/img/stylesheet-branch-open.png");
   m_extendedImage  = tmp->scaled(QSize(6,7), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}


void ExtensionButton::paintEvent(QPaintEvent *evt)
{
   QPushButton::paintEvent(evt);
   QPainter p(this);
   int vCenter;
   p.save();

   if(isChecked() == extCollapsed)
   {
      vCenter = (size().height() - m_collapsedImage.size().height())/2;
      p.drawImage(QPoint(5, vCenter), m_collapsedImage);
   }
   else
   {
      vCenter = (size().height() - m_extendedImage.size().height())/2;
      p.drawImage(QPoint(5,vCenter), m_extendedImage);

   }

   QFontMetrics metrics(p.font());
   vCenter = (size().height() + metrics.ascent() - metrics.descent())/2;
   p.drawText(QPoint(20,vCenter),m_text); //Simple Text.
   p.restore();
}

}//terbit
