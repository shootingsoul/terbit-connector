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

/** This class takes a widget in its constructor and places that widget inside
 * another Widget that provides a button at the top for "expanding", and
 * "collapsing" the widget below it.
 * The button is a subclass of QPushButton with the paint event overwritten to
 * include graphics in addition to the button text.  The graphic is a small
 * icon (a triangle) to indicate whether the widget is expanded or collapsed.
 * The more button is used to keep the expanded/collapsed state by making it
 * a "checkable" button.  When btn->getChecked() is true, then the state is
 * "expanded".
 * Usage Example:
 *
 *    pWidg = new QWidget(this); // Create the widget to be hidden
      pLayout = new QVBoxLayout;
      pLayout->addWidget(new QLabel(tr("Item 1")));
      pLayout->addWidget(new QLabel(tr("Item 2")));
      ...

      pWidg->setLayout(pLayout);

      // create extension widget by passing the widget above
      pExtWidg = new ExtensionWidget(this, "My Widget", pWidg));

      pLayout = new QVBoxLayout;
      pLayout->addWidget(pExtWidg);
      pLayout->setContentsMargins(0,0,0,0);
      setLayout(pLayout);


      // Set size policy on top level widget (e.g. the widget added to the dock
      // window
      setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

      // place extension widget where desired, such as a dock window
      addDockWidget(Qt::LeftDockWidgetArea, pExtWidg, Qt::Vertical);
*/
#pragma once
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QImage>

namespace terbit
{


class ExtensionButton : public QPushButton
{
   Q_OBJECT
public:
   ExtensionButton(QWidget *parent =  0);
   void SetText(const QString& str){m_text = str;}

protected:
   void paintEvent(QPaintEvent* evt);

private:
   const bool extExpanded = true;
   const bool extCollapsed = !extExpanded;
   QString m_text;
   QImage m_collapsedImage;
   QImage m_extendedImage;
};

class ExtensionWidget : public QWidget
{
   Q_OBJECT
public:
   explicit ExtensionWidget(QWidget *parent = 0, QString name = "", QWidget* ext = 0);
   void SetToolTipText(const QString str) {if(m_moreButton) m_moreButton->setToolTip(str);}
   QWidget* GetWidget(void) const {return m_ExtensionWidget;}
   void SetCollapsed(bool c) {extend(!c);}
   bool GetCollapsed(void) const;

signals:

private slots:
   void extend(bool extend);

private:
   ExtensionButton *m_moreButton = NULL;
   QVBoxLayout *m_layout         = NULL;
   QWidget *m_ExtensionWidget    = NULL;

};
}// terbit

