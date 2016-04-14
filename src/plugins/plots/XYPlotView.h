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

#include <QAction>
#include <QContextMenuEvent>
#include "connector-core/WorkspaceDockWidget.h"

namespace terbit
{

class XYPlotViewArea;
class XYPlot;
class ZoomScrollbar;
class XYSeries;
class DataSet;

class XYPlotView: public WorkspaceDockWidget
{
   Q_OBJECT
public:
   XYPlotView(XYPlot* plot);

   ZoomScrollbar* GetScrollbarX() { return m_scrollX; }
   ZoomScrollbar* GetScrollbarY() { return m_scrollY; }

   void RebuildAxis();
   void Replot();

   void dragEnterEvent(QDragEnterEvent *event);
   void dropEvent(QDropEvent *event);

private slots:
   void OnScrollbarX();
   void OnScrollbarY();

   void OnContextDataSourceHide();
   void OnContextDataSourceShow();
   void OnContextDataSourceRemove();
   void OnContextDataSourceAdd();
   void OnContextSizeToFit();
   void OnContextGridLinesX();
   void OnContextGridLinesY();
   void OnContextProperties();

protected:
   void contextMenuEvent(QContextMenuEvent *event);

private:
   XYPlotView(const XYPlotView& o); //disable copy ctor

   void SetupKeyboardShortcuts();

   XYPlot *m_plot;
   XYPlotViewArea *m_viewArea;

   ZoomScrollbar *m_scrollX, *m_scrollY;

   //context menu helper classes
   class DataSetAction : public QAction
   {
   public:
      DataSetAction(DataSet* s, QObject* parent) : QAction(parent), dataSet(s) {}
      DataSet* dataSet;
   };
   class XYSeriesAction : public QAction
   {
   public:
      XYSeriesAction(XYSeries* s, QObject* parent) : QAction(parent), series(s) {}
      XYSeries* series;
   };
};


class XYPlotViewArea : public QWidget
{
    Q_OBJECT

public:
   XYPlotViewArea(QWidget* parent, XYPlot* plot);
   ~XYPlotViewArea();

   void RebuildAxis();

protected:
   void paintEvent(QPaintEvent *event);
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *);
   void resizeEvent(QResizeEvent *event);
   void wheelEvent(QWheelEvent *);

private:
   enum State
   {
      STATE_IDLE,
      STATE_PANNING
   };

   void BuildAxis(int plotWidth, int plotHeight);

   State m_state;
   XYPlot *m_plot;
   QImage* m_backgroundAndAxis;
   QPoint m_lastMousePos;
   bool m_rebuildAxis;
};


}
