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

#include <list>
#include <QFontMetrics>
#include "XYSeriesRenderer.h"

class QPainter;

namespace terbit
{

class XYPlot;

class XYPlotRenderer
{
public:
   XYPlotRenderer(XYPlot* plot);
   virtual ~XYPlotRenderer();

   void RenderBackgroundAndAxis(QPainter* painter, int plotWidth, int plotHeight);
   void RenderSeriesArea(QPainter* painter);
   void RenderOverlay(QPainter* painter, int mouseX, int mouseY);
   bool ScreenToDataPoint(int posX, int posY, double& dataX, double& dataY);
   bool SeriesAreaContainsPoint(int posX, int posY);
   void GetScales(double& scaleX, double& scaleY);

private:

   class DisplayTick
   {
   public:
      DisplayTick(double v, const QString& s) : value(v), text(s) {}
      double value;
      QString text;
   };

   void DrawVeritcalText(QPainter *painter, int x, int y, const QString &text);
   void CalcSeriesRectX(int plotWidth, const QFontMetrics& fm, const std::list<DisplayTick>& displayValuesY);
   void CalcSeriesRectY(int plotHeight, const QFontMetrics& fm);
   void CalcDisplayTicks(std::list<DisplayTick>& displayValues, int padFontHeight, int pixels, double start, double end);

   XYPlot* m_plot;
   XYSeriesRenderArea m_seriesArea;
   int m_textBuffer, m_tickLen, m_seriesBorder;
};

}
