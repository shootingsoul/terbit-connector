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
#include "XYPlotRenderer.h"
#include "XYPlot.h"
#include "XYSeriesRenderer.h"

namespace terbit
{

XYPlotRenderer::XYPlotRenderer(XYPlot* pPlot) : m_plot(pPlot), m_textBuffer(4), m_tickLen(6), m_seriesBorder(3)
{
}

XYPlotRenderer::~XYPlotRenderer()
{
}

void XYPlotRenderer::RenderBackgroundAndAxis(QPainter* painter, int plotWidth, int plotHeight)
{
   //clean slate for whole control
   QColor color;
   color= QApplication::palette().color(QPalette::Window); //system background color
   painter->setBrush(color);
   painter->setPen(color);
   painter->drawRect(0, 0, plotWidth, plotHeight);

   XYSeriesRenderArea& a = m_seriesArea;

   bool hasData = m_plot->GetHasData();

   QFontMetrics fm(painter->font());
   int textBuffer = m_textBuffer;
   int textBufferHalf = m_textBuffer / 2;
   int tickLen = m_tickLen;
   int padFontHeight = fm.height()+textBuffer;

   if (hasData)
   {
      m_plot->GetVisibleX(a.startX, a.endX);
      a.rangeX = a.endX - a.startX;
      m_plot->GetVisibleY(a.startY, a.endY);
      a.rangeY = a.endY - a.startY;
   }

   std::list<DisplayTick> displayValuesY, displayValuesX;
   CalcSeriesRectY(plotHeight,fm);
   if (hasData)
   {
      CalcDisplayTicks(displayValuesY, padFontHeight, a.rectH, a.startY, a.endY);
   }
   CalcSeriesRectX(plotWidth, fm, displayValuesY);
   if (hasData)
   {
      CalcDisplayTicks(displayValuesX, padFontHeight, a.rectW, a.startX, a.endX);
   }

   //now draw everything . . .
   int x1, y1, x2, y2, tickX, tickY;

   //series area clean slate
   color = m_plot->GetSeriesAreaBackgroundColor();
   painter->setBrush(color);
   painter->setPen(color);
   painter->drawRect(a.rectX - m_seriesBorder, a.rectY - m_seriesBorder, a.rectW + m_seriesBorder*2, a.rectH + m_seriesBorder*2);

   QPen gridLinePen(Qt::lightGray);
   gridLinePen.setStyle(Qt::DotLine);

   color= QApplication::palette().color(QPalette::Text);
   painter->setBrush(color);
   painter->setPen(color);

   //title
   if (m_plot->GetShowTitle() && m_plot->GetName().length() > 0)
   {
      painter->drawText((plotWidth-fm.width(m_plot->GetName()))/2, padFontHeight - textBufferHalf, m_plot->GetName());
   }

   //x-axis
   x1 = a.rectX;
   y1 = a.rectY+a.rectH+1+m_seriesBorder;
   x2 = a.rectX+a.rectW;
   y2 = y1;
   painter->drawLine(x1-m_seriesBorder,y1,x2+m_seriesBorder,y2);
   if (m_plot->GetLabelX().length() > 0)
   {
      painter->drawText(x1+(x2-x1-fm.width(m_plot->GetLabelX()))/2, plotHeight - textBufferHalf, m_plot->GetLabelX());
   }

   //x ticks
   for(std::list<DisplayTick>::iterator tick = displayValuesX.begin(); tick != displayValuesX.end(); ++tick)
   {
      tickX = ScaleDataToLogical(tick->value,a.rectX,a.rectW,a.startX,a.rangeX);
      painter->drawLine(tickX,y1,tickX,y1+tickLen);
      painter->drawText(tickX-fm.width(tick->text)/2,y1+tickLen+padFontHeight-textBufferHalf, tick->text);

      if (m_plot->GetGridLinesX())
      {
         painter->setPen(gridLinePen);
         painter->drawLine(tickX,a.rectY-m_seriesBorder, tickX, a.rectY+a.rectH+m_seriesBorder);
         painter->setPen(color); //reset pen back
      }
   }

   //y-axis
   x1 = a.rectX - 1 - m_seriesBorder;
   y1 = a.rectY;
   x2 = x1;
   y2 = a.rectY+a.rectH+1; //draw corner point, so add one
   painter->drawLine(x1,y1-m_seriesBorder,x2,y2+m_seriesBorder);
   --y2; //remove corner point
   if (m_plot->GetLabelY().length() > 0)
   {
      DrawVeritcalText(painter,padFontHeight - textBufferHalf, y2-(y2-y1-fm.width(m_plot->GetLabelY()))/2,m_plot->GetLabelY());
   }

   //y ticks
   for(std::list<DisplayTick>::iterator tick = displayValuesY.begin(); tick != displayValuesY.end(); ++tick)
   {
      tickY = ScaleDataToLogicalReverse(tick->value,a.rectY,a.rectH,a.startY,a.rangeY);
      painter->drawLine(x1-tickLen,tickY,x1,tickY);
      painter->drawText(x1-fm.width(tick->text)-tickLen-textBufferHalf,tickY+fm.height()/2, tick->text);

      if (m_plot->GetGridLinesY())
      {
         painter->setPen(gridLinePen);
         painter->drawLine(a.rectX-m_seriesBorder, tickY, a.rectX+a.rectW+m_seriesBorder, tickY);
         painter->setPen(color); //reset pen back
      }
   }   
}

void XYPlotRenderer::CalcSeriesRectY(int plotHeight, const QFontMetrics& fm)
{
   //all padding will be calced with extra pixel because series area is inclusive for drawing series area
   int padX, padTop;

   int tickLen = m_tickLen;
   int padFontHeight = fm.height()+m_textBuffer;

   //x-axis (show values plus axis label plus line (always show line))
   if (m_plot->GetLabelX().length() > 0)
   {
      padX = padFontHeight * 2 + tickLen + 2;
   }
   else
   {
      padX = padFontHeight + tickLen + 2;
   }
   padX += m_seriesBorder;

   //Top (value overlay plus title)
   if (m_plot->GetShowTitle() && m_plot->GetName().length() > 0)
   {
      padTop = padFontHeight * 2 + 1;
   }
   else
   {
      padTop = padFontHeight + 1;
   }
   padTop += m_seriesBorder;

   m_seriesArea.rectY = padTop;
   m_seriesArea.rectH = plotHeight - padTop - padX;
}

void XYPlotRenderer::CalcSeriesRectX(int plotWidth, const QFontMetrics& fm, const std::list<DisplayTick>& displayValuesY)
{
   //all padding will be calced with extra pixel because series area is inclusive for drawing series area
   int padY, padY2;

   int yValueWidth(0), y2ValueWidth(0);
   int textBuffer = m_textBuffer;
   int tickLen = m_tickLen;
   int padFontHeight = fm.height()+textBuffer;

   for (std::list<DisplayTick>::const_iterator valueY = displayValuesY.begin() ; valueY != displayValuesY.end(); ++valueY)
   {
      int w = fm.width(valueY->text);
      if (w > yValueWidth)
      {
         yValueWidth = w;
      }
   }

   //y-axis (show values plus axis label plus line (always show line))
   if (m_plot->GetLabelY().length() > 0)
   {
      padY = padFontHeight + yValueWidth + textBuffer + tickLen + 2;
   }
   else
   {
      padY = yValueWidth + textBuffer + tickLen + 2;
   }
   padY += m_seriesBorder;

   //y2-axis (show values plus axis label and line) - all optional
   if (m_plot->GetLabelY2().length() > 0)
   {
      padY2 = padFontHeight + y2ValueWidth + textBuffer + tickLen + 2;
   }
   else
   {
      padY2 = y2ValueWidth + padFontHeight + 1; //default pad to something even-though nothing will show
   }
   padY2 += m_seriesBorder;

   m_seriesArea.rectX = padY;
   m_seriesArea.rectW  = plotWidth - padY - padY2;
}

void XYPlotRenderer::CalcDisplayTicks(std::list<DisplayTick>& displayValues, int padFontHeight, int pixels, double start, double end)
{
   int count;
   double step;

   //determine in general how many tick marks to display, 1 = max density
   //density setting basically means show every J values based on text height for increment
   double maxDensity = 4.0; //lower number means more dense, higher number is less dense, must be >= 1

   count = ceil((double)pixels/padFontHeight/maxDensity); //density 1 means put as many ticks without overlaping text display
   step = (end - start)/count;

   //magnitude factor e.g. 0.01, 1, 10, 100, 1000
   double magnitudeFactor = qPow(10.0, qFloor(qLn(step)/qLn(10.0)));
   double tickStepMantissa = step/magnitudeFactor;
   if (tickStepMantissa < 5)
   {
     //round digit after decimal point to 0.5
     step = (int)(tickStepMantissa*2)/2.0*magnitudeFactor;
   }
   else
   {
     //round to first digit in multiples of 2
     step = (int)(tickStepMantissa/2.0)*2.0*magnitudeFactor;
   }

   int decimals = CalculateDecimalPlaces(step);

   //now we have the step to use
   //figure out first value and step through
   //only include values within the start/end range (inclusive)
   double pos = start-fmod(start, step);

   if (pos < start)
   {
      pos += step;
   }

   while (pos <= end)
   {
      if (decimals > 0)
      {
         //force all numbers to display with same precision
         displayValues.push_back(DisplayTick(pos, QString::number(pos,'f',decimals)));
      }
      else
      {
         displayValues.push_back(DisplayTick(pos, QString::number(pos)));
      }
      pos += step;
   }
}

void XYPlotRenderer::RenderSeriesArea(QPainter* painter)
{
   //draw white background
   XYSeriesRenderArea& a = m_seriesArea;

   if (m_plot->GetHasData())
   {
      //rendering may need to draw lines to points outside series area (e.g. zoomed)
      //but only paint portion of lines inside series area
      painter->setClipRect(a.rectX,a.rectY,a.rectW,a.rectH);

      std::list<XYSeries*>& list = m_plot->GetSeriesList();
      std::list<XYSeries*>::iterator its;

      //render the series
      //qint64 ms = QDateTime::currentMSecsSinceEpoch();

      for(its=list.begin(); its!=list.end(); ++its)
      {
         XYSeries* series = (*its);

         if (series->GetShowOnPlot() && series->HasData())
         {                        
            DataSet* X = series->GetX();
            DataSet* Y = series->GetY();

            //get bounding indicies that overlap the visible range
            //assumes X data is ordered
            size_t start, end;
            if (X->BoundingIndicies(a.startX,a.endX,start, end))
            {
               series->GetRenderer()->Render(painter, a, X, Y, start, end);
            }
         }
      }

      //done clipping
      painter->setClipping(false);


      //finally done processing new data . . . update GUI plot latency
//      ms = QDateTime::currentMSecsSinceEpoch() - ms;
//      QString displayLatency = QObject::tr("%1 ms").arg(QString::number(ms));
//      painter->setPen(Qt::black);
//      painter->drawText(a.rectW+a.rectX-50,a.rectY-2,displayLatency);
   }
}
bool XYPlotRenderer::ScreenToDataPoint(int mouseX, int mouseY, double& dataX, double& dataY)
{
   //screen position to interpolated data point (not necessarily exact data point)
   bool res;
   if (mouseX >=  m_seriesArea.rectX && mouseX <= m_seriesArea.rectX + m_seriesArea.rectW &&
       mouseY >=  m_seriesArea.rectY && mouseY <= m_seriesArea.rectY + m_seriesArea.rectH &&
       m_plot->GetHasData())
   {
      dataX = ScaleLogicalToData(mouseX-m_seriesArea.rectX,0,m_seriesArea.rectW,m_seriesArea.startX,m_seriesArea.rangeX);
      dataY = ScaleLogicalReverseToData(mouseY-m_seriesArea.rectY,0,m_seriesArea.rectH,m_seriesArea.startY,m_seriesArea.rangeY);
      res = true;
   }
   else
   {
      res = false;
   }
   return res;
}
void XYPlotRenderer::GetScales(double& scaleX, double& scaleY)
{
   //data per pixel
   scaleX = m_seriesArea.rangeX/m_seriesArea.rectW;
   scaleY = m_seriesArea.rangeY/m_seriesArea.rectH;
}

bool XYPlotRenderer::SeriesAreaContainsPoint(int mouseX, int mouseY)
{
   return (mouseX >=  m_seriesArea.rectX && mouseX <= m_seriesArea.rectX + m_seriesArea.rectW &&
              mouseY >=  m_seriesArea.rectY && mouseY <= m_seriesArea.rectY + m_seriesArea.rectH);
}

void XYPlotRenderer::RenderOverlay(QPainter* painter, int mouseX, int mouseY)
{
   if (mouseX >=  m_seriesArea.rectX && mouseX <= m_seriesArea.rectX + m_seriesArea.rectW &&
       mouseY >=  m_seriesArea.rectY && mouseY <= m_seriesArea.rectY + m_seriesArea.rectH &&
       m_plot->GetHasData())
   {

      QString s;
      QFontMetrics fm(painter->font());
      double dataX = ScaleLogicalToData(mouseX-m_seriesArea.rectX,0,m_seriesArea.rectW,m_seriesArea.startX,m_seriesArea.rangeX);
      int x = mouseX;
      int y;

      //draw veritcal line across entire series based on mouse position
      y = m_seriesArea.rectY+m_seriesArea.rectH + m_seriesBorder;
      painter->setPen(Qt::lightGray);
      painter->drawLine(x,m_seriesArea.rectY-m_seriesBorder, x, y);

      //display Y values at mouse x
      int eh;
      eh = fm.height();
      int circleDiameter, circleY;
      y = m_seriesArea.rectY - 5 - m_seriesBorder;
      circleDiameter = eh/2;
      circleY = m_seriesArea.rectY - m_seriesBorder - circleDiameter - 6;
      x = m_seriesArea.rectX+2;

      std::list<XYSeries*>& list = m_plot->GetSeriesList();
      std::list<XYSeries*>::iterator its;

      for(its=list.begin(); its!=list.end(); ++its)
      {
         if ((*its)->GetShowOnPlot())
         {
            QColor color = (*its)->GetColor();
            QBrush b;
            b.setColor(color);
            b.setStyle(Qt::SolidPattern);
            painter->setPen(color);
            painter->setBrush(b);

            painter->drawEllipse(x, circleY,circleDiameter,circleDiameter);
            x += circleDiameter + 5;

            double pointX, pointY;
            if ((*its)->HasData() && ClosestDataPoint((*its)->GetX(), (*its)->GetY(), dataX, pointX, pointY))
            {
               //for mouse pos values, don't use scientific notation, display full value
               s = QObject::tr("%1, %2").arg(DoubleToStringComplete(pointX)).arg(DoubleToStringComplete(pointY));
            }
            else
            {
               s = QObject::tr("No Data");
            }

            painter->drawText(x,y,s);
            x += fm.width(s) + 5;
         }
      }
   }
}

void XYPlotRenderer::DrawVeritcalText(QPainter *painter, int x, int y, const QString &text)
{
   //draw text to read from bottom to top
    painter->save();
    painter->translate(x, y);
    painter->rotate(270); // or 90 . . .
    painter->drawText(0, 0, text);
    painter->restore();
}

}
