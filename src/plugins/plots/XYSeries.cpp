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
#include "XYSeries.h"
#include "XYPlot.h"
#include "XYSeriesRenderer.h"
#include "connector-core/Workspace.h"
#include "XYPlotPropertiesView.h"

namespace terbit
{

XYSeries::XYSeries(Workspace* workspace, DataSet* X, DataSet* Y, XYPlot* plot, bool managedX)
   : QObject(), m_workspace(workspace), m_X(X), m_Y(Y), m_plot(plot), m_managedX(managedX), m_renderer(NULL), m_showOnPlot(false), m_lastNewDataCounterX(0), m_lastNewDataCounterY(0)
{   
   m_color = m_plot->GetDefaultSeriesColor();
   m_plot->GetSeriesList().push_back(this);

   m_showOnPlot = true;
   m_renderer = new XYSeriesRenderer(this);

   connect(X,SIGNAL(NameChanged(DataClass*)),this, SLOT(OnBufferNameChanged(DataClass*)));
   connect(Y,SIGNAL(NameChanged(DataClass*)),this, SLOT(OnBufferNameChanged(DataClass*)));
}

XYSeries::~XYSeries()
{
   delete m_renderer;
   if (m_managedX)
   {
      m_workspace->DeleteInstance(m_X->GetAutoId());
   }
}

void XYSeries::SetShowOnPlot(bool showOnPlot)
{
   m_showOnPlot = showOnPlot;
   if (m_plot->GetPropertiesView())
   {
      m_plot->GetPropertiesView()->UpdateSeriesDisplay(this);
   }

}

bool XYSeries::TestPlotNewData(DataClass* source)
{
   bool res = false;
   if (m_showOnPlot)
   {
      DataSet* Y = GetY();
      DataSet* X = GetX();
      if (Y == source || (X == source && Y->GetHasData()))
      {
         //see if we have to reconfigure X
         if (m_managedX && (X->GetCount() != Y->GetCount() || X->GetFirstIndex() != Y->GetFirstIndex() ))
         {
            X->CreateBufferAsIndex(Y->GetFirstIndex(), Y->GetCount());
         }
         res = true;
      }
   }

   return res;
}

void XYSeries::SetNewDataCounters(size_t newDataCounterY, size_t newDataCounterX)
{
   //track last data counter of what we plotted
   m_lastNewDataCounterY = newDataCounterY;
   m_lastNewDataCounterX = newDataCounterX;
}

void XYSeries::SetColor(const QColor& color)
{
   m_color = color;   
   if (m_plot->GetPropertiesView())
   {
      m_plot->GetPropertiesView()->UpdateSeriesDisplay(this);
   }
}

QString XYSeries::GetDescription()
{
   QString s;
   if (m_Y != NULL)
   {
      s = m_Y->GetName();
   }
   return s;
}

void XYSeries::OnBufferNameChanged(DataClass*)
{
   //no need to replot for name change
   if (m_plot->GetPropertiesView())
   {
      m_plot->GetPropertiesView()->UpdateSeriesDisplay(this);
   }

}

QObject *XYSeries::CreateScriptWrapper(QJSEngine *se)
{
   return new XYSeriesSW(se,this);
}



ScriptDocumentation *BuildScriptDocumentationXYSeries()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->SetName(QObject::tr("Series"));
   d->SetSummary(QObject::tr("A series on a line plot."));

   d->AddScriptlet(new Scriptlet(QObject::tr("GetShowOnPlot"), "GetShowOnPlot();",QObject::tr("Returns boolean if the series is visible on the plot.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetShowOnPlot"), "SetShowOnPlot(value);",QObject::tr("Set boolean option to hide or show series on the plot display.  This does not remove the series.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("Hide"), "Hide();",QObject::tr("Hides the series on the plot display. This does not remove the series.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("SetRGB"), "SetRGB(red, green, blue);",QObject::tr("Sets the color values for the red, green and blue components.")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetRed"), "GetRed();",QObject::tr("Returns the red component of the color for the series")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetGreen"), "GetGreen();",QObject::tr("Returns the green component of the color for the series")));
   d->AddScriptlet(new Scriptlet(QObject::tr("GetBlue"), "GetBlue();",QObject::tr("Returns the blue component of the color for the series")));

   return d;
}


XYSeriesSW::XYSeriesSW(QJSEngine *se, XYSeries *series) : m_scriptEngine(se), m_series(series)
{
}

bool XYSeriesSW::GetShowOnPlot()
{
   return m_series->GetShowOnPlot();
}

void XYSeriesSW::SetShowOnPlot(bool show)
{
   m_series->SetShowOnPlot(show);
}

void XYSeriesSW::Hide()
{
   m_series->SetShowOnPlot(false);
}

void XYSeriesSW::SetRGB(int red, int green, int blue)
{
   QColor c(red,green, blue);
   m_series->SetColor(c);
}

int XYSeriesSW::GetRed()
{
   return m_series->GetColor().red();
}

int XYSeriesSW::GetGreen()
{
   return m_series->GetColor().green();
}

int XYSeriesSW::GetBlue()
{
   return m_series->GetColor().blue();
}

}
