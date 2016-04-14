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
#include <QObject>
#include <QtWidgets>
#include <QDockWidget>
#include <QColor>
#include <QString>
#include "connector-core/DataSet.h"
#include "connector-core/Block.h"

#include <QJSValue>

namespace terbit
{

class Workspace;
class XYSeries;
class XYPlotView;
class XYPlotRenderer;
class XYPlotPropertiesView;

static const char* XYPLOT_TYPENAME = "xyplot";

class XYPlot : public Block
{
   Q_OBJECT
public:
   XYPlot();
   virtual ~XYPlot();

   virtual bool Init();
   virtual bool InteractiveInit();
   virtual bool InteractiveFocus();

   QObject* CreateScriptWrapper(QJSEngine* se);

   void ApplyInputDataClass(DataClass* dc);

   XYSeries* AddSeries(DataSet* X, DataSet* Y);


   std::list<XYSeries*>& GetSeriesList() { return m_list; }

   virtual void Refresh();

   XYPlotPropertiesView* GetPropertiesView() { return m_propertiesView; }

   //for use by XYPlotView only . . .
   XYPlotRenderer* GetRenderer() { return m_renderer; }

   QColor GetDefaultSeriesColor();

   void GetLimitsX(double& min, double& max);
   void GetVisibleX(double& start, double& end);

   void GetLimitsY(double& min, double& max);
   void GetVisibleY(double& start, double& end);

   bool GetHasData() { return m_hasData; }
   bool GetAutoScale() { return m_autoScale; }
   void SetAutoScale(bool autoScale) { m_autoScale = autoScale; }

   void SetName(const QString& name);
   bool GetShowTitle() { return m_showTitle; }
   void SetShowTitle(bool show);

   const QString& GetLabelX() { return m_labelX; }
   const QString& GetLabelY() { return m_labelY; }
   const QString& GetLabelY2() { return m_labelY2; }
   void SetLabelX(const QString& value);
   void SetLabelY(const QString& value);
   void SetLabelY2(const QString& value);

   void SizeToFit();
   void SetGridLinesX(bool value);
   bool GetGridLinesX() { return m_gridLinesX; }
   void SetGridLinesY(bool value);
   bool GetGridLinesY() { return m_gridLinesY; }

   bool IsDataSetOnPlot(DataClassAutoId_t bufferId);

   virtual void GetDirectDependencies(std::list<DataClass *> &dependsOn);

   void BuildRestoreScript(ScriptBuilder& script, const QString& variableName);

   //plot scrolling/moving/panning/zooming . . . .
   void ZoomIn(double dataX, double dataY) { ZoomByPoint(dataX, dataY, 0.95); }
   void ZoomOut(double dataX, double dataY) { ZoomByPoint(dataX, dataY, 1.05); }
   void ZoomInY(double point) { ZoomY(point, 0.95); }
   void ZoomInX(double point) { ZoomX(point, 0.95); }
   void ZoomOutY(double point) { ZoomY(point, 1.05); }
   void ZoomOutX(double point) { ZoomX(point, 1.05); }
   bool Pan(double deltaX, double deltaY);

   const QColor& GetSeriesAreaBackgroundColor() { return m_seriesAreaBackgroundColor; }

   bool SetLimitStartX(double value);
   bool SetLimitEndX(double value);
   bool SetLimitStartY(double value);
   bool SetLimitEndY(double value);
   void SetLimitsX(double min, double max);
   void SetLimitsY(double min, double max);

   bool SetStartX(double value);
   bool SetEndX(double value);
   bool SetStartY(double value);
   bool SetEndY(double value);

   //TODO: refactor and make private
   void UpdatedVisibleX();
   void UpdatedVisibleY();

   public slots:
   void StepPreviousX();
   void StepPreviousY();
   void StepNextX();
   void StepNextY();
   void PagePreviousX();
   void PagePreviousY();
   void PageNextX();
   void PageNextY();
   void MoveFirstX();
   void MoveFirstY();
   void MoveLastX();
   void MoveLastY();
   void StepPreviousStartHandleX();
   void StepPreviousStartHandleY();
   void StepNextEndHandleX();
   void StepNextEndHandleY();
   void StepNextStartHandleX();
   void StepPreviousEndHandleX();
   void StepPreviousEndHandleY();
   void StepNextStartHandleY();
   void ZoomIn();
   void ZoomOut();

   bool ShowPropertiesView();
   void ClosePropertiesView();

signals:
   void ShowView();
   void CloseView();
   void SeriesAdded(XYSeries* series);
   bool RemoveSeries(DataSet* buf);


private slots:
   void OnShowView();
   void OnCloseView();
   void OnViewClosed();

   void OnPropertiesViewClosed();

   void OnBeforeBufferRemovedSlot(DataClass* dc);
   void OnNewData(DataClass* source);

   bool OnRemoveSeries(DataSet* buf);

private:
   XYPlot(const XYPlot& o); //disable copy ctor
   void CalcDataMinMax(double& minY, double& maxY, double& minX, double& maxX);
   void AutoScale(bool initScale);
   void ZoomByPoint(double dataX, double dataY, double deltaPercent);
   void ZoomX(double point, double deltaPercent);
   void ZoomY(double point, double deltaPercent);
   void SetVisibleX(double start, double end);
   void SetVisibleY(double start, double end);


   std::list<XYSeries*> m_list;
   XYPlotView* m_view;
   XYPlotPropertiesView* m_propertiesView;
   XYPlotRenderer* m_renderer;

   QString m_labelX, m_labelY, m_labelY2;

   double m_minY, m_maxY, m_minX, m_maxX;
   bool m_hasData, m_autoScale, m_showTitle;
   bool m_gridLinesX, m_gridLinesY;
   int m_defaultColorIndex;
   QColor m_seriesAreaBackgroundColor;
};

ScriptDocumentation *BuildScriptDocumentationXYPlot();

class XYPlotSW : public BlockSW
{
   Q_OBJECT
public:
   XYPlotSW(QJSEngine* se, XYPlot* plot);

   Q_INVOKABLE void SetShowTitle(bool value);
   Q_INVOKABLE bool GetShowTitle();
   Q_INVOKABLE void SetLabelX(const QString& value);
   Q_INVOKABLE QString GetLabelX();
   Q_INVOKABLE void SetLabelY(const QString& value);
   Q_INVOKABLE QString GetLabelY();
   Q_INVOKABLE void SetStartX(double value);
   Q_INVOKABLE double GetStartX();
   Q_INVOKABLE void SetEndX(double value);
   Q_INVOKABLE double GetEndX();
   Q_INVOKABLE void SetStartY(double value);
   Q_INVOKABLE double GetStartY();
   Q_INVOKABLE void SetEndY(double value);
   Q_INVOKABLE double GetEndY();
   Q_INVOKABLE void SetAutoIncreaseRange(bool value);
   Q_INVOKABLE bool GetAutoIncreaseRange();
   Q_INVOKABLE void SetRangeX(double min, double max);
   Q_INVOKABLE void SetRangeY(double min, double max);
   Q_INVOKABLE double GetMinX();
   Q_INVOKABLE double GetMaxX();
   Q_INVOKABLE double GetMinY();
   Q_INVOKABLE double GetMaxY();
   Q_INVOKABLE bool GetGridLinesY();
   Q_INVOKABLE void SetGridLinesY(bool value);
   Q_INVOKABLE bool GetGridLinesX();
   Q_INVOKABLE void SetGridLinesX(bool value);

   Q_INVOKABLE void ShowPropertiesWindow();
   Q_INVOKABLE void ClosePropertiesWindow();

   Q_INVOKABLE int GetSeriesCount();
   Q_INVOKABLE QJSValue GetSeries(int index);
   Q_INVOKABLE QJSValue AddSeries(const QJSValue& valueX, const QJSValue& valueY);
   Q_INVOKABLE void RemoveSeries(const QJSValue& dataSet);

private:
   XYPlot* m_plot;
};

}
