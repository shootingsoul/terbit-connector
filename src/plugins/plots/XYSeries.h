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

#include <QColor>
#include <QString>
#include "connector-core/DataSet.h"
#include "XYPlot.h"

namespace terbit
{

class XYSeriesRenderer;
class XYSeriesRenderArea;
class Workspace;

class XYSeries : public QObject
{
   Q_OBJECT
public:
   XYSeries(Workspace* workspace, DataSet* X, DataSet* Y, XYPlot* plot, bool managedX);
   virtual ~XYSeries();

   XYPlot* GetPlot() { return m_plot; }

   bool GetShowOnPlot() { return m_showOnPlot; }
   void SetShowOnPlot(bool showOnPlot);


   void SetColor(const QColor& color);
   const QColor& GetColor() { return m_color; }

   QString GetDescription();

   XYSeriesRenderer* GetRenderer() { return m_renderer; }

   //VirtualDevice* GetVirtualDevice();
   DataSet* GetY(){return m_Y;}
   DataSet* GetX(){return m_X;}
   bool GetManagedX() { return m_managedX; }

   bool HasData() {return (m_Y)?m_Y->GetHasData():false; }

   QObject* CreateScriptWrapper(QJSEngine* se);

   bool TestPlotNewData(DataClass* source);
   void SetNewDataCounters(size_t newDataCounterY, size_t newDataCounterX);

private slots:
   void OnBufferNameChanged(DataClass*);


private:
   XYSeries(const XYSeries& o); //disable copy ctor

   Workspace* m_workspace;
   DataSet* m_X;
   DataSet* m_Y;
   XYPlot* m_plot;
   bool m_managedX;
   XYSeriesRenderer* m_renderer;
   size_t m_lastNewDataCounterX, m_lastNewDataCounterY;

   QColor m_color;
   bool m_showOnPlot;

};

ScriptDocumentation *BuildScriptDocumentationXYSeries();

class XYSeriesSW : public QObject
{
   Q_OBJECT
public:
   XYSeriesSW(QJSEngine* se, XYSeries* series);

   Q_INVOKABLE bool GetShowOnPlot();
   Q_INVOKABLE void SetShowOnPlot(bool show);
   Q_INVOKABLE void Hide();
   Q_INVOKABLE void SetRGB(int red, int green, int blue);
   Q_INVOKABLE int GetRed();
   Q_INVOKABLE int GetGreen();
   Q_INVOKABLE int GetBlue();

private:
   QJSEngine* m_scriptEngine;
   XYSeries* m_series;
};


}

