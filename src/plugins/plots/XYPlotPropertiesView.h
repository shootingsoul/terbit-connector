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

#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include "connector-core/WorkspaceDockWidget.h"

namespace terbit
{

class XYPlot;
class XYSeries;

class XYPlotPropertiesView: public WorkspaceDockWidget
{
   Q_OBJECT
public:
   XYPlotPropertiesView(XYPlot* plot);

   void SetName(const QString& name) { m_title->setText(name); UpdateWindowTitle(); }
   void SetLabelY(const QString& value) { m_labelY->setText(value); }
   void SetLabelX(const QString& value) { m_labelX->setText(value); }
   void SetShowTitle(bool value) { m_showTitle->setChecked(value); }
   void SetStartEndX(double start, double end);
   void SetStartEndY(double start, double end);
   void SetLimitStartEndX(double start, double end);
   void SetLimitStartEndY(double start, double end);

   void UpdateSeriesDisplay(XYSeries *series);

public slots:
   void RemoveSeries(XYSeries* series);
   void AddSeries(XYSeries* series);

   void TitleEditFinished();
   void LabelYEditFinished();
   void LabelXEditFinished();
   void StartXEditFinished();
   void EndXEditFinished();
   void StartYEditFinished();
   void EndYEditFinished();
   void ShowTitleChanged(int);
   void AutoScaleChanged(int);

   void LimitStartXEditFinished();
   void LimitEndXEditFinished();
   void LimitStartYEditFinished();
   void LimitEndYEditFinished();


private:
   XYPlotPropertiesView(const XYPlotPropertiesView& o); //disable copy ctor

   void UpdateWindowTitle();
   void BuildSeriesDisplay();


   XYPlot *m_plot;
   QCheckBox* m_showTitle, *m_autoScale;
   QLineEdit *m_title, *m_labelY, *m_labelX, *m_startX, *m_endX, *m_startY, *m_endY;
   QLineEdit *m_limitStartX, *m_limitEndX, *m_limitStartY, *m_limitEndY;
   QVBoxLayout* m_seriesArea;

};


class XYPlotPropertiesSeriesWidget : public QWidget
{
   Q_OBJECT

public:
   XYPlotPropertiesSeriesWidget(XYSeries* series);

   XYSeries* GetSeries() { return m_series; }
   void UpdateValueDisplay();

private slots:
   void OnSelectColor();
   void OnShowOnCheckBoxToggle(bool value);
   void OnRemove();
private:
   class SeriesButton : public QPushButton
   {
   public:
      SeriesButton(XYSeries* series);
   protected:
      virtual void paintEvent(QPaintEvent* event);
   private:
      XYSeries* m_series;
   };

   XYSeries* m_series;
   QCheckBox* m_showOnPlot;
   SeriesButton* m_button;
   QLabel* m_description;
};


}
