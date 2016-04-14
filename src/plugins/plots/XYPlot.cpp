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
#include "XYPlot.h"
#include "XYPlotView.h"
#include "XYPlotPropertiesView.h"
#include "XYSeries.h"
#include "XYPlotRenderer.h"
#include "connector-core/Workspace.h"
#include "tools/widgets/ZoomScrollbar.h"
#include "connector-core/LogDL.h"
#include "tools/Tools.h"

namespace terbit
{

XYPlot::XYPlot() : Block(), m_view(NULL), m_propertiesView(NULL), m_autoScale(true), m_showTitle(false)
{
   m_renderer = new XYPlotRenderer(this);

   m_minY= m_maxY = m_minX= m_maxX = 0;
   m_hasData = false;
   m_gridLinesX = m_gridLinesY = true;
   m_defaultColorIndex = 0;
   m_seriesAreaBackgroundColor = QApplication::palette().color(QPalette::Base);

   connect(this, SIGNAL(ShowView()),this, SLOT(OnShowView()));
   connect(this, SIGNAL(CloseView()),this, SLOT(OnCloseView()));

   connect(this,SIGNAL(RemoveSeries(DataSet*)),this,SLOT(OnRemoveSeries(DataSet*)));
}

XYPlot::~XYPlot()
{
   //make sure views are closed
   ClosePropertiesView();
   if (m_view)
   {
      disconnect(m_view,SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      delete m_view;
   }

   delete m_renderer;

   std::list<XYSeries*>::iterator it;
   for(it=m_list.begin(); it!=m_list.end(); ++it)
   {
      XYSeries* series = *it;
      disconnect(series->GetX(),SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));
      disconnect(series->GetY(),SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));
      delete series;
   }
}

bool XYPlot::Init()
{
   ShowView();
   return true;
}

bool XYPlot::InteractiveInit()
{
return true;
}

bool XYPlot::InteractiveFocus()
{
   GetWorkspace()->WidgetAlert(m_view);
   return true;
}

void XYPlot::OnNewData(DataClass* source)
{
   //NOTE: already in GUI thread
   //if one of the buffers with new data is displayed on the plot, then replot entire plot
   //replot entire plot to avoid dealing with series that overlap each other, autoscaling, etc.

   if (m_view != NULL)
   {
      //if plot is showing the data set, then replot
      std::list<XYSeries*>::iterator it;
      for(it=m_list.begin(); it!=m_list.end(); ++it)
      {
         XYSeries* series = (*it);

         if (series->TestPlotNewData(source))
         {
            //if X has new data, then redo the scales
            //don't redo scales if Y has new data because then scales can jump around
            bool initScale = (!m_hasData || series->GetX() == source);
            m_hasData = true;
            if (m_autoScale)
            {
               AutoScale(initScale);
            }
            else if (initScale)
            {
               m_view->RebuildAxis();
            }

            m_view->Replot();
            break;
         }
      }
   }
}

void XYPlot::OnBeforeBufferRemovedSlot(DataClass *dc)
{
   if (RemoveSeries(static_cast<DataSet*>(dc)))
   {
      Refresh();
   }
}

void XYPlot::ApplyInputDataClass(DataClass* dc)
{
   if (dc && dc->IsDataSet())
   {
      DataSet* buf = static_cast<DataSet*>(dc);
      DataSet *X, *Y;
      Y = buf;
      X = buf->GetIndexDataSet();

      if (X != NULL)
      {
         if (m_list.size() == 0)
         {
            m_labelX = X->GetName();
            m_labelY = Y->GetName();
            SetName(buf->GetName());
         }
      }
      else
      {
         if (m_list.size() == 0)
         {
            SetName(buf->GetName());
         }
      }

      if (AddSeries(X, Y) != NULL)
      {
         Refresh();
         Block::ApplyInputDataClass(dc);
      }
   }
   else if (dc && dc->IsBlock())
   {
      Block* block = static_cast<Block*>(dc);
      for(auto& i : block->GetOutputs())
      {
         DataClass* output = i.second;
         if (output->GetPublicScope() && output->IsDataSet())
         {
            ApplyInputDataClass(output);
         }
      }
   }
}


XYSeries* XYPlot::AddSeries(DataSet *X, DataSet *Y)
{
   bool managedX = false;

   if (!X)
   {
      managedX = true;
      X = GetWorkspace()->CreateDataSet(this,false);
      X->CreateBufferAsIndex(Y->GetFirstIndex(), Y->GetCount());
      X->SetHasData(true);
   }

   //create new series, will be managed by plot . . . no worries . . .
   XYSeries* series = NULL;

   //return existing series (if any or create new one)
   for(std::list<XYSeries*>::iterator i = m_list.begin(); i != m_list.end(); ++i)
   {
      if (((XYSeries*)(*i))->GetY() == Y)
      {
         series = *i;
         break;
      }
   }

   if (series == NULL)
   {
      connect(X, SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
      connect(Y, SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
      connect(X,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));
      connect(Y,SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));

      series = new XYSeries(GetWorkspace(), X, Y, this, managedX);

      bool initScale = !m_hasData;
      if (!m_hasData && Y->GetHasData())
      {
         m_hasData = true;
      }
      if (Y->GetHasData() && m_autoScale)
      {
         AutoScale(initScale);
      }
      emit SeriesAdded(series);
   }
   else
   {
      if (managedX)
      {
         GetWorkspace()->DeleteInstance(X->GetAutoId());
      }
   }

   return series;
}

bool XYPlot::OnRemoveSeries(DataSet *buf)
{
   bool removalMade = false;
   XYSeries* series, *current;
   series = NULL;
   std::list<XYSeries*>::iterator it;

   DataClassAutoId_t id = buf->GetAutoId();

   bool hasData = false;
   do
   {
      series = NULL;
      for(it=m_list.begin(); it!=m_list.end(); ++it)
      {
         current = *it;
         if (current->GetY()->GetAutoId() == id || current->GetX()->GetAutoId() == id)
         {
            series = (*it);
         }
         else
         {
            hasData = hasData || current->GetY()->GetHasData();
         }
      }
      if (series != NULL)
      {
         disconnect(series->GetX(), SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
         disconnect(series->GetY(), SIGNAL(NewData(DataClass*)), this, SLOT(OnNewData(DataClass*)));
         disconnect(series->GetX(),SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));
         disconnect(series->GetY(),SIGNAL(BeforeDeletion(DataClass*)),this,SLOT(OnBeforeBufferRemovedSlot(DataClass*)));
         m_hasData = hasData;
         m_list.remove(series);
         if (m_propertiesView)
         {
            m_propertiesView->RemoveSeries(series);
         }
         delete series;
         removalMade = true;
      }
   }
   while (series != NULL);

   return removalMade;
}


bool XYPlot::IsDataSetOnPlot(DataClassAutoId_t bufferId)
{
   bool onPlot = false;
   std::list<XYSeries*>::iterator s;
   for(s=m_list.begin(); s!=m_list.end() && !onPlot; ++s)
   {
      if ((*s)->GetY()->GetAutoId() == bufferId)
      {
         onPlot = true;
         break;
      }
   }

   return onPlot;
}

void XYPlot::GetDirectDependencies(std::list<DataClass *> &dependsOn)
{
   for(XYSeries* s : m_list)
   {
      dependsOn.push_back(s->GetY());
      if (!s->GetManagedX())
      {
         dependsOn.push_back(s->GetX());
      }
   }
}

void XYPlot::AutoScale(bool initScale)
{   
   ZoomScrollbar *xScroll, *yScroll;
   bool yScaleUpdated, xScaleUpdated;
   double yMin, yMax, yEnd, yStart, xMin, xMax, xEnd, xStart;

   yScaleUpdated = xScaleUpdated = false;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();
   CalcDataMinMax(yMin, yMax, xMin, xMax);

   if (std::isinf(yMin) || std::isinf(yMax) || std::isinf(xMin) || std::isinf(xMax))
   {
      LogWarning2(GetType()->GetLogCategory(),GetName(),tr("The data contains an infinite value.  The plot scales will not be update, and the plot may not display properly."));
      return;
   }

   if (!initScale)
   {
      //ongoing autoscale
      //only expand scale, never shrink display, try to keep same size
      //only do this if they zoomed in, otherwise keep it at 100%

      if (yMin < m_minY)
      {
         m_minY = yMin;
         yScaleUpdated = true;
      }
      if (yMax > m_maxY)
      {
         m_maxY = yMax;
         yScaleUpdated = true;
      }
      if (yScaleUpdated)
      {
         //keep current area visible or at 100%
         if (yScroll->FullDisplay())
         {
            yStart = m_minY;
            yEnd = m_maxY;
         }
         else
         {
            yStart = yScroll->GetVisibleStart();
            yEnd = yScroll->GetVisibleEnd();
         }
      }

      if (xMin < m_minX)
      {
         m_minX = xMin;
         xScaleUpdated = true;
      }
      if (xMax > m_maxX)
      {
         m_maxX = xMax;
         xScaleUpdated = true;
      }
      if (xScaleUpdated)
      {
         //keep current area visible or at 100%
         if (xScroll->FullDisplay())
         {
            xStart = m_minX;
            xEnd = m_maxX;
         }
         else
         {
            xStart = xScroll->GetVisibleStart();
            xEnd = xScroll->GetVisibleEnd();
         }
      }
   }
   else
   {
      //init/re-init autoscale to keep tight fit
      //tight fit
      m_minY = yStart = yMin;
      m_maxY = yEnd = yMax;
      m_minX = xStart = xMin;
      m_maxX = xEnd = xMax;

      //test for flat-line and single values
      if (m_minY == m_maxY)
      {
         m_maxY += .00000001;
         yEnd = m_maxY;
      }
      if (m_minX == m_maxX)
      {
         m_maxX += .00000001;
         xEnd = m_maxX;
      }


      yScaleUpdated = xScaleUpdated = true;
   }

   if (xScaleUpdated)
   {
      ZoomScrollbar *s = m_view->GetScrollbarX();
      s->SetScrollbar(m_minX, m_maxX, xStart, xEnd);
      s->update();
      if (m_propertiesView)
      {
         m_propertiesView->SetLimitStartEndX(m_minX,m_maxX);
         m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());         
      }
   }
   if (yScaleUpdated)
   {
      ZoomScrollbar *s = m_view->GetScrollbarY();
      s->SetScrollbar(m_minY, m_maxY, yStart, yEnd);
      s->update();
      if (m_propertiesView)
      {
         m_propertiesView->SetLimitStartEndY(m_minY,m_maxY);
         m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());         
      }
   }
   if (xScaleUpdated || yScaleUpdated)
   {
      m_view->RebuildAxis();
   }
}
void XYPlot::SetVisibleX(double start, double end)
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   s->SetScrollbar(m_minX, m_maxX, start, end);
   s->update();
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndX(start, end);
   }
}

void XYPlot::SetVisibleY(double start, double end)
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   s->SetScrollbar(m_minY, m_maxY, start, end);
   s->update();
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndY(start, end);
   }
}

void XYPlot::UpdatedVisibleX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   s->update();
   m_view->RebuildAxis();
   Refresh(); //update display here . . . only called from keyboard/GUI interaction functions
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
   }
}
void XYPlot::UpdatedVisibleY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   s->update();
   m_view->RebuildAxis();
   Refresh(); //update display here . . . only called from keyboard/GUI interaction functions
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
   }
}
void XYPlot::ZoomIn()
{
   ZoomScrollbar *xScroll, *yScroll;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();

   ZoomIn((xScroll->GetVisibleEnd()-xScroll->GetVisibleStart())/2+xScroll->GetVisibleStart(),(yScroll->GetVisibleEnd()-yScroll->GetVisibleStart())/2+yScroll->GetVisibleStart());
   Refresh(); //update display here . . . only called from keyboard/GUI interaction functions
}

void XYPlot::ZoomOut()
{
   ZoomScrollbar *xScroll, *yScroll;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();

   ZoomOut((xScroll->GetVisibleEnd()-xScroll->GetVisibleStart())/2+xScroll->GetVisibleStart(),(yScroll->GetVisibleEnd()-yScroll->GetVisibleStart())/2+yScroll->GetVisibleStart());
   Refresh(); //update display here . . . only called from keyboard/GUI interaction functions
}

void XYPlot::ZoomByPoint(double dataX, double dataY, double deltaPercent)
{
   ZoomScrollbar *xScroll, *yScroll;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();

   double posPercentX, posPercentY, startX, startY;
   double visibleRangeX, visibleRangeY;
   visibleRangeX = xScroll->GetVisibleEnd()-xScroll->GetVisibleStart();
   visibleRangeY = yScroll->GetVisibleEnd()-yScroll->GetVisibleStart();
   posPercentX = (dataX-xScroll->GetVisibleStart())/visibleRangeX;
   posPercentY = (dataY-yScroll->GetVisibleStart())/visibleRangeY;

   //zoom in or out
   visibleRangeX *= deltaPercent;
   visibleRangeY *= deltaPercent;

   //keep position in same location
   startX = dataX-posPercentX*visibleRangeX;
   startY = dataY-posPercentY*visibleRangeY;

   SetVisibleX(startX,startX+visibleRangeX);
   SetVisibleY(startY,startY+visibleRangeY);

   m_view->RebuildAxis();
}

void XYPlot::ZoomY(double point, double deltaPercent)
{
   ZoomScrollbar *scroll;
   scroll = m_view->GetScrollbarY();

   double posPercent, start;
   double visibleRange;
   visibleRange = scroll->GetVisibleEnd()-scroll->GetVisibleStart();
   posPercent = (point-scroll->GetVisibleStart())/visibleRange;

   //zoom in or out
   visibleRange *= deltaPercent;

   //keep position in same location
   start = point-posPercent*visibleRange;

   SetVisibleY(start,start+visibleRange);

   m_view->RebuildAxis();
}

void XYPlot::ZoomX(double point, double deltaPercent)
{
   ZoomScrollbar *scroll;
   scroll = m_view->GetScrollbarX();

   double posPercent, start;
   double visibleRange;
   visibleRange = scroll->GetVisibleEnd()-scroll->GetVisibleStart();
   posPercent = (point-scroll->GetVisibleStart())/visibleRange;

   //zoom in or out
   visibleRange *= deltaPercent;

   //keep position in same location
   start = point-posPercent*visibleRange;

   SetVisibleX(start,start+visibleRange);

   m_view->RebuildAxis();
}


bool XYPlot::Pan(double deltaX, double deltaY)
{
   //pan in data units
   ZoomScrollbar *xScroll, *yScroll;

   bool changeMade = false;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();

   if (deltaX != 0)
   {
      SetVisibleX(xScroll->GetVisibleStart()+deltaX,xScroll->GetVisibleEnd()+deltaX);
      changeMade = true;
   }

   if (deltaY != 0)
   {
      SetVisibleY(yScroll->GetVisibleStart()+deltaY,yScroll->GetVisibleEnd()+deltaY);
      changeMade = true;
   }

   if (changeMade)
   {
      m_view->RebuildAxis();
   }
   return changeMade;
}

bool XYPlot::SetLimitStartX(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (value <= m_maxX)
   {
      m_minX = value;
      //scrollbar adjusts visible if needed
      s->SetScrollbar(m_minX, m_maxX, s->GetVisibleStart(), s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
         m_propertiesView->SetLimitStartEndX(m_minX, m_maxX);
      }
      res = true;      
   }
   return res;
}

bool XYPlot::SetLimitEndX(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (value >= m_minX)
   {
      m_maxX = value;
      //scrollbar adjusts visible if needed
      s->SetScrollbar(m_minX, m_maxX, s->GetVisibleStart(),  s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
         m_propertiesView->SetLimitStartEndX(m_minX, m_maxX);
      }
      res = true;
   }
   return res;
}

bool XYPlot::SetLimitStartY(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (value <= m_maxY)
   {
      m_minY = value;
      //scrollbar adjusts visible if needed
      s->SetScrollbar(m_minY, m_maxY, s->GetVisibleStart(), s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
         m_propertiesView->SetLimitStartEndY(m_minY, m_maxY);
      }
      res = true;
   }
   return res;

}

bool XYPlot::SetLimitEndY(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (value >= m_minY)
   {
      m_maxY = value;
      //scrollbar adjusts visible if needed
      s->SetScrollbar(m_minY, m_maxY, s->GetVisibleStart(), s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
         m_propertiesView->SetLimitStartEndY(m_minY, m_maxY);
      }
      res = true;
   }
   return res;
}

void XYPlot::SetLimitsX(double min, double max)
{
   m_minX = min;
   m_maxX = max;
   ZoomScrollbar *s = m_view->GetScrollbarX();
   //scrollbar adjusts visible if needed
   s->SetScrollbar(m_minX, m_maxX, s->GetVisibleStart(), s->GetVisibleEnd());
   s->update();
   m_view->RebuildAxis();
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
      m_propertiesView->SetLimitStartEndX(m_minX, m_maxX);
   }
}

void XYPlot::SetLimitsY(double min, double max)
{
   m_minY = min;
   m_maxY = max;
   ZoomScrollbar *s = m_view->GetScrollbarY();
   //scrollbar adjusts visible if needed
   s->SetScrollbar(m_minY, m_maxY, s->GetVisibleStart(), s->GetVisibleEnd());
   s->update();
   m_view->RebuildAxis();
   if (m_propertiesView)
   {
      m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
      m_propertiesView->SetLimitStartEndY(m_minY, m_maxY);
   }
}

bool XYPlot::SetStartX(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (value == s->GetVisibleStart() )
   {
      res = true; //nothing to update
   }
   else if (value >= m_minX && value <= m_maxX && value < s->GetVisibleEnd())
   {
      s->SetScrollbar(m_minX, m_maxX, value, s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
      }
      res = true;
   }

   return res;
}

bool XYPlot::SetEndX(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (value == s->GetVisibleEnd() )
   {
      res = true; //nothing to update
   }
   else if (value >= m_minX && value <= m_maxX && value > s->GetVisibleStart())
   {
      s->SetScrollbar(m_minX, m_maxX, s->GetVisibleStart(), value);
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndX(s->GetVisibleStart(), s->GetVisibleEnd());
      }
      res = true;
   }

   return res;
}

bool XYPlot::SetStartY(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (value == s->GetVisibleStart() )
   {
      res = true; //nothing to update
   }
   else if (value >= m_minY && value <= m_maxY && value < s->GetVisibleEnd())
   {
      s->SetScrollbar(m_minY, m_maxY, value, s->GetVisibleEnd());
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
      }
      res = true;
   }

   return res;
}

bool XYPlot::SetEndY(double value)
{
   bool res = false;
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (value == s->GetVisibleEnd() )
   {
      res = true; //nothing to update
   }
   else if (value >= m_minY && value <= m_maxY && value > s->GetVisibleStart())
   {
      s->SetScrollbar(m_minY, m_maxY, s->GetVisibleStart(), value);
      s->update();
      m_view->RebuildAxis();
      if (m_propertiesView)
      {
         m_propertiesView->SetStartEndY(s->GetVisibleStart(), s->GetVisibleEnd());
      }
      res = true;
   }

   return res;

}

void XYPlot::StepPreviousX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MovePreviousStep())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepNextX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MoveNextStep())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepNextY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MoveNextStep())
   {
      UpdatedVisibleY();
   }

}
void XYPlot::StepPreviousY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MovePreviousStep())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::PagePreviousX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MovePreviousPage())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::PageNextX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MoveNextPage())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::PageNextY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MoveNextPage())
   {
      UpdatedVisibleY();
   }

}
void XYPlot::PagePreviousY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MovePreviousPage())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::MoveFirstX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MoveFirst())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::MoveLastX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->MoveLast())
   {
      UpdatedVisibleX();
   }

}
void XYPlot::MoveLastY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MoveLast())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::MoveFirstY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->MoveFirst())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::StepPreviousStartHandleX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->ExtendStart())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepNextEndHandleX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->ExtendEnd())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepNextEndHandleY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->ExtendEnd())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::StepPreviousStartHandleY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->ExtendStart())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::StepNextStartHandleX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->ShrinkStart())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepPreviousEndHandleX()
{
   ZoomScrollbar *s = m_view->GetScrollbarX();
   if (s->ShrinkEnd())
   {
      UpdatedVisibleX();
   }
}
void XYPlot::StepPreviousEndHandleY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->ShrinkEnd())
   {
      UpdatedVisibleY();
   }
}
void XYPlot::StepNextStartHandleY()
{
   ZoomScrollbar *s = m_view->GetScrollbarY();
   if (s->ShrinkStart())
   {
      UpdatedVisibleY();
   }
}

void XYPlot::OnShowView()
{
   if (m_view)
   {
      m_view->raise();
   }
   else
   {      
      m_view = new XYPlotView(this);
      connect(m_view, SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      GetWorkspace()->AddDockWidget(m_view);
      m_view->setFocus();
   }
}

void XYPlot::OnCloseView()
{
   if (m_view)
   {
      disconnect(m_view, SIGNAL(destroyed()),this,SLOT(OnViewClosed()));
      delete m_view;
      m_view = NULL;
      //remove ourself
      GetWorkspace()->DeleteInstance(this->GetAutoId());
   }
}

void XYPlot::OnViewClosed()
{
   m_view = NULL;
   //remove ourself
   GetWorkspace()->DeleteInstance(this->GetAutoId());
}

bool XYPlot::ShowPropertiesView()
{
   if (m_propertiesView)
   {
      m_propertiesView->raise();
   }
   else
   {
      m_propertiesView = new XYPlotPropertiesView(this);
      connect(m_propertiesView, SIGNAL(destroyed()),this,SLOT(OnPropertiesViewClosed()));
      GetWorkspace()->AddDockWidget(m_propertiesView);
   }
   return true;
}
void XYPlot::OnPropertiesViewClosed()
{
   m_propertiesView = NULL;
}
void XYPlot::ClosePropertiesView()
{
   if (m_propertiesView)
   {
      disconnect(m_propertiesView, SIGNAL(destroyed()),this,SLOT(OnPropertiesViewClosed()));
      delete m_propertiesView;
      m_propertiesView = NULL;
   }
}

void XYPlot::Refresh()
{
   if (m_view)
   {
      m_view->Replot();
   }
   if (m_propertiesView)
   {
      m_propertiesView->update();
   }
}

void XYPlot::SizeToFit()
{
   AutoScale(true);
}

void XYPlot::SetGridLinesX(bool value)
{
   m_gridLinesX = value;
   if (m_view)
   {
      m_view->RebuildAxis();
   }
}
void XYPlot::SetGridLinesY(bool value)
{
   m_gridLinesY = value;
   if (m_view)
   {
      m_view->RebuildAxis();
   }
}

QColor XYPlot::GetDefaultSeriesColor()
{
   QColor c;
   //default colour . . .
   m_defaultColorIndex = m_defaultColorIndex % 8;
   switch (m_defaultColorIndex)
   {
      case 0:
         c = QApplication::palette().color(QPalette::Highlight);
         break;
      case 1:
         c = QColor(Qt::green);
         break;
      case 2:
         c = QColor(148,0,211); //purple
         break;
      case 3:
         c = QColor(Qt::cyan);
         break;
      case 4:
         c = QColor(Qt::darkYellow);
         break;
      case 5:
         c = QColor(255,102,0); //orange
         break;
      case 6:
         c = QColor(Qt::red);
         break;
      default:
         c = QColor(Qt::magenta);
   }
   ++m_defaultColorIndex;

   return c;
}


void XYPlot::GetVisibleX(double& start, double& end)
{
   start = m_view->GetScrollbarX()->GetVisibleStart();
   end = m_view->GetScrollbarX()->GetVisibleEnd();
}

void XYPlot::GetVisibleY(double& start, double& end)
{
   start = m_view->GetScrollbarY()->GetVisibleStart();
   end = m_view->GetScrollbarY()->GetVisibleEnd();
}

void XYPlot::GetLimitsY(double& min, double& max)
{
   min = m_minY;
   max = m_maxY;
}

void XYPlot::GetLimitsX(double& min, double& max)
{
   min = m_minX;
   max = m_maxX;
}

void XYPlot::CalcDataMinMax(double& minY, double& maxY, double& minX, double& maxX)
{
   bool first = true;

   TerbitValue minValue, maxValue;
   minValue.SetDataType(TERBIT_DOUBLE);
   maxValue.SetDataType(TERBIT_DOUBLE);

   for(XYSeries* its: m_list)
   {
      if (its->GetShowOnPlot() && its->HasData())
      {
         double min, max;
         DataSet* X, * Y;
         X = its->GetX();
         Y = its->GetY();
         if (first)
         {
            X->CalculateMinMax(minValue, maxValue);
            minX = *((double*)minValue.GetValue());
            maxX = *((double*)maxValue.GetValue());
            Y->CalculateMinMax(minValue, maxValue);
            minY = *((double*)minValue.GetValue());
            maxY = *((double*)maxValue.GetValue());
            first = false;
         }
         else
         {
            X->CalculateMinMax(minValue, maxValue);
            min = *((double*)minValue.GetValue());
            max = *((double*)maxValue.GetValue());
            if (min < minX)
            {
               minX = min;
            }
            if (max > maxX)
            {
               maxX = max;
            }
            Y->CalculateMinMax(minValue, maxValue);
            min = *((double*)minValue.GetValue());
            max = *((double*)maxValue.GetValue());
            if (min < minY)
            {
               minY = min;
            }
            if (max > maxY)
            {
               maxY = max;
            }
         }
      }
   }
}

void XYPlot::SetName(const QString &name)
{
   Block::SetName(name);
   if (m_view)
   {
      m_view->setWindowTitle(name);
      if (m_showTitle)
      {
         m_view->RebuildAxis();
      }
   }
   if (m_propertiesView)
   {
      m_propertiesView->SetName(name);
   }
}

void XYPlot::SetShowTitle(bool show)
{
   m_showTitle = show;
   if (m_view)
   {
      m_view->RebuildAxis();
   }
   if (m_propertiesView)
   {
      m_propertiesView->SetShowTitle(show);
   }
}

void XYPlot::SetLabelX(const QString &value)
{
   m_labelX = value;
   if (m_view)
   {
      m_view->RebuildAxis();
   }
   if (m_propertiesView)
   {
      m_propertiesView->SetLabelX(value);
   }
}

void XYPlot::SetLabelY(const QString &value)
{
   m_labelY = value;
   if (m_view)
   {
      m_view->RebuildAxis();
   }
   if (m_propertiesView)
   {
      m_propertiesView->SetLabelY(value);
   }
}

QObject *XYPlot::CreateScriptWrapper(QJSEngine *se)
{
   return new XYPlotSW(se,this);
}

void XYPlot::BuildRestoreScript(ScriptBuilder& script, const QString& variableName)
{
   script.add(QString("var X;\nvar Y;\nvar series;"));

   //add  series . . . find series from provider (if from a provider), or from global list
   for(std::list<XYSeries*>::iterator its=m_list.begin(); its!=m_list.end(); ++its)
   {
      XYSeries* series = *its;      
      script.add(QString("Y = %1.Find(%2);").arg(script.VARIABLE_WORKSPACE).arg(ScriptEncode(series->GetY()->GetUniqueId())));
      if (series->GetManagedX())
      {
         script.add(QString("X = null;\n"));
      }
      else
      {
         script.add(QString("X = %1.Find(%2);").arg(script.VARIABLE_WORKSPACE).arg(ScriptEncode(series->GetX()->GetUniqueId())));
      }

      script.add("if (undefined != Y)\n{");
      script.add(QString("series = %1.AddSeries(X, Y);").arg(variableName));
      if (!series->GetShowOnPlot())
      {
         script.add(QString("series.Hide());"));
      }
      const QColor& c = series->GetColor();
      script.add(QString("series.SetRGB(%1, %2, %3);").arg(c.red()).arg(c.green()).arg(c.blue()));
      script.add("}");
   }

   //set properties
   if (m_labelX.length() > 0)
   {
      script.add(QString("%1.SetLabelX(%2);").arg(variableName).arg(ScriptEncode(m_labelX)));
   }
   if (m_labelY.length() > 0)
   {
      script.add(QString("%1.SetLabelY(%2);").arg(variableName).arg(ScriptEncode(m_labelY)));
   }

   script.add(QString("%1.SetShowTitle(%2);").arg(variableName).arg(m_showTitle));
   script.add(QString("%1.SetGridLinesX(%2);").arg(variableName).arg(m_gridLinesX));
   script.add(QString("%1.SetGridLinesY(%2);").arg(variableName).arg(m_gridLinesY));

   script.add(QString("%1.SetAutoIncreaseRange(%2);").arg(variableName).arg(m_autoScale));
   if (!m_autoScale)
   {
      script.add(QString("%1.SetRangeX(%2,%3);").arg(variableName).arg(DoubleToStringComplete(m_minX)).arg(DoubleToStringComplete(m_maxX)));
      script.add(QString("%1.SetRangeY(%2,%3);").arg(variableName).arg(DoubleToStringComplete(m_minY)).arg(DoubleToStringComplete(m_maxY)));
   }

   ZoomScrollbar* xScroll, *yScroll;
   xScroll = m_view->GetScrollbarX();
   yScroll = m_view->GetScrollbarY();
   script.add(QString("%1.SetStartX(%2);").arg(variableName).arg(DoubleToStringComplete(xScroll->GetVisibleStart())));
   script.add(QString("%1.SetEndX(%2);").arg(variableName).arg(DoubleToStringComplete(xScroll->GetVisibleEnd())));
   script.add(QString("%1.SetStartY(%2);").arg(variableName).arg(DoubleToStringComplete(yScroll->GetVisibleStart())));
   script.add(QString("%1.SetEndY(%2);").arg(variableName).arg(DoubleToStringComplete(yScroll->GetVisibleEnd())));

   script.add(QString("%1.Refresh();").arg(variableName));

   if (m_propertiesView)
   {
      script.add(QString("%1.ShowPropertiesWindow();").arg(variableName));
   }
}

ScriptDocumentation *BuildScriptDocumentationXYPlot()
{
   ScriptDocumentation* d = BuildScriptDocumentationBlock();

   d->SetSummary(QObject::tr("The line plot."));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetShowTitle"), "SetShowTitle(value);",QObject::tr("Boolean option to display the title on the plot itself.  The title already displays on the window frame.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetShowTitle"), "GetShowTitle();",QObject::tr("The title of the plot.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetLabelX"), "SetLabelX(value);",QObject::tr("Set the X-axis label.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetLabelX"), "GetLabelX();",QObject::tr("Returns the X-axis label.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetLabelY"), "SetLabelY(value);",QObject::tr("Set the Y-axis label.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetLabelY"), "GetLabelY();",QObject::tr("Returns the Y-axis label.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetStartX"), "SetStartX(value);",QObject::tr("Set the starting value for the X-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetStartX"), "GetStartX();",QObject::tr("Returns the starting value for the X-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetEndX"), "SetEndX(value);",QObject::tr("Set the ending value for the X-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetEndX"), "GetEndX();",QObject::tr("Returns the ending value for the X-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetStartY"), "SetStartY(value);",QObject::tr("Set the starting value for the Y-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetStartY"), "GetStartY();",QObject::tr("Returns the starting value for the Y-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetEndY"), "SetEndY(value);",QObject::tr("Set the ending value for the Y-axis visible area.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetEndY"), "GetEndY();",QObject::tr("Returns the ending value for the Y-axis visible area.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetAutoIncreaseRange"), "SetAutoIncreaseRange(value);",QObject::tr("Boolean option to increase the range size based on data values.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetAutoIncreaseRange"), "GetAutoIncreaseRange();",QObject::tr("Returns boolean option to increase the range size based on data values.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetRangeX"), "SetRangeX(min, max);",QObject::tr("Set the min/max range values for the X-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetMinX"), "GetMinX();",QObject::tr("The minimum value for the X-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetMaxX"), "GetMaxX();",QObject::tr("The maximum value for the X-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetRangeY"), "SetRangeX(min, max);",QObject::tr("Set the min/max range values for the Y-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetMinY"), "GetMinY();",QObject::tr("The minimum value for the Y-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetMaxY"), "GetMaxY();",QObject::tr("The maximum value for the Y-axis.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("SetGridLinesY"), "SetGridLinesY(value);",QObject::tr("Boolean option to display gridlines for the Y-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetGridLinesY"), "GetGridLinesY();",QObject::tr("Returns boolean option to display gridlines for the Y-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetGridLinesX"), "SetGridLinesX(value);",QObject::tr("Boolean option to display gridlines for the X-axis.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetGridLinesX"), "GetGridLinesX();",QObject::tr("Returns boolean option to display gridlines for the X-axis.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("ShowPropertiesWindow"), "ShowPropertiesWindow();",QObject::tr("Show the plot properties window.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("ClosePropertiesWindow"), "ClosePropertiesWindow();",QObject::tr("Close the plot properties window.")));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetSeriesCount"), "GetSeriesCount();",QObject::tr("return the number of series on the plot.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetSeries"), "GetSeries(index);",QObject::tr("Returns a reference to the series.  The index is 0-based.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("AddSeries"), "AddSeries(X, Y);",QObject::tr("Add a series to the plot and returns a refernce to the series.  The X and Y arguments reference data sets or unique id strings.  The X data set may be null.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("RemoveSeries"), "RemoveSeries(dataSet);",QObject::tr("Remove a series from the plot that matches the given data set.  The data set may be a reference or unique id string.")));


   d->AddSubDocumentation(BuildScriptDocumentationXYSeries());

   return d;
}

XYPlotSW::XYPlotSW(QJSEngine* se, XYPlot *plot) : BlockSW(se,plot), m_plot(plot)
{
}

void XYPlotSW::SetShowTitle(bool value)
{
   m_plot->SetShowTitle(value);
}

bool XYPlotSW::GetShowTitle()
{
   return m_plot->GetShowTitle();
}

void XYPlotSW::SetLabelX(const QString &value)
{
   m_plot->SetLabelX(value);
}

QString XYPlotSW::GetLabelX()
{
   return m_plot->GetLabelX();
}

void XYPlotSW::SetLabelY(const QString &value)
{
   m_plot->SetLabelY(value);
}

QString XYPlotSW::GetLabelY()
{
   return m_plot->GetLabelY();
}

void XYPlotSW::SetStartX(double value)
{
   m_plot->SetStartX(value);
}

double XYPlotSW::GetStartX()
{
   double start, end;
   m_plot->GetVisibleX(start,end);
   return start;
}

void XYPlotSW::SetEndX(double value)
{
   m_plot->SetEndX(value);
}

double XYPlotSW::GetEndX()
{
   double start, end;
   m_plot->GetVisibleX(start,end);
   return end;
}

void XYPlotSW::SetStartY(double value)
{
   m_plot->SetStartY(value);
}

double XYPlotSW::GetStartY()
{
   double start, end;
   m_plot->GetVisibleY(start,end);
   return start;

}

void XYPlotSW::SetEndY(double value)
{
   m_plot->SetEndY(value);
}

double XYPlotSW::GetEndY()
{
   double start, end;
   m_plot->GetVisibleY(start,end);
   return end;
}

void XYPlotSW::SetAutoIncreaseRange(bool value)
{
   m_plot->SetAutoScale(value);
}

bool XYPlotSW::GetAutoIncreaseRange()
{
   return m_plot->GetAutoScale();
}

void XYPlotSW::SetRangeX(double min, double max)
{
   m_plot->SetLimitsX(min,max);
}

void XYPlotSW::SetRangeY(double min, double max)
{
   m_plot->SetLimitsY(min,max);
}

double XYPlotSW::GetMinX()
{
   double start, end;
   m_plot->GetLimitsX(start,end);
   return start;
}

double XYPlotSW::GetMaxX()
{
   double start, end;
   m_plot->GetLimitsX(start,end);
   return end;
}

double XYPlotSW::GetMinY()
{
   double start, end;
   m_plot->GetLimitsY(start,end);
   return start;
}

double XYPlotSW::GetMaxY()
{
   double start, end;
   m_plot->GetLimitsY(start,end);
   return end;
}

bool XYPlotSW::GetGridLinesY()
{
   return m_plot->GetGridLinesY();
}

void XYPlotSW::SetGridLinesY(bool value)
{
   m_plot->SetGridLinesY(value);
}

bool XYPlotSW::GetGridLinesX()
{
   return m_plot->GetGridLinesX();
}

void XYPlotSW::SetGridLinesX(bool value)
{
   m_plot->SetGridLinesX(value);
}
void XYPlotSW::ShowPropertiesWindow()
{
   m_plot->ShowPropertiesView();
}

void XYPlotSW::ClosePropertiesWindow()
{
   m_plot->ClosePropertiesView();
}

int XYPlotSW::GetSeriesCount()
{
   return m_plot->GetSeriesList().size();
}

QJSValue XYPlotSW::GetSeries(int index)
{
   QJSValue res;
   XYSeries* series = NULL;
   int i = 0;
   for(auto it : m_plot->GetSeriesList())
   {
      if (i == index)
      {
         series = it;
         res = m_scriptEngine->newQObject(series->CreateScriptWrapper(m_scriptEngine));
         break;
      }
      ++i;
   }
   if (series == NULL)
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(),m_dataClass->GetName(),tr("Script XYPlot GetSeries index out of range."));
   }

   return res;
}

QJSValue XYPlotSW::AddSeries(const QJSValue& valueX, const QJSValue& valueY)
{
   QJSValue res;
   DataSetSW* dswX = static_cast<DataSetSW*>(valueX.toQObject());
   DataSetSW* dswY = static_cast<DataSetSW*>(valueY.toQObject());
   if (dswY)
   {
      DataSet* Y = dswY->GetDataSet();
      DataSet* X;
      if (dswX)
      {
        X = dswX->GetDataSet();
      }
      else
      {
         X = NULL;
      }

      XYSeries* series = m_plot->AddSeries(X,Y);
      if (series)
      {
         res = m_scriptEngine->newQObject(series->CreateScriptWrapper(m_scriptEngine));
      }
   }
   else
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(),m_dataClass->GetName(),tr("Script XYPlot CreateSeries invalid argument"));
   }
   return res;
}

void XYPlotSW::RemoveSeries(const QJSValue &dataSet)
{
   DataClass* ds = m_plot->GetWorkspace()->FindInstance(dataSet);

   if (ds && ds->IsDataSet())
   {
      m_plot->RemoveSeries(static_cast<DataSet*>(ds));
   }
   else
   {
      LogError2(m_dataClass->GetType()->GetLogCategory(),m_dataClass->GetName(),tr("Script XYPlot RemoveSeries invalid argument or series not found"));
   }
}

}
