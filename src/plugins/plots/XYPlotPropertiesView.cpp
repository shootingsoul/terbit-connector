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
#include <QGridLayout>
#include <QTabWidget>
#include <QScrollArea>
#include "XYPlot.h"
#include "XYPlotPropertiesView.h"
#include "XYSeries.h"
#include "tools/Tools.h"

namespace terbit
{



XYPlotPropertiesView::XYPlotPropertiesView(XYPlot* plot)
   : WorkspaceDockWidget(plot,""), m_plot(plot)
{
   connect(m_plot,SIGNAL(SeriesAdded(XYSeries*)),this,SLOT(AddSeries(XYSeries*)));

   //properties grid
   QLabel* l;
   QGridLayout* grid = new QGridLayout();
   int row = 0;

   m_showTitle = new QCheckBox(tr("Show Title"));
   m_showTitle->setChecked(m_plot->GetShowTitle());
   connect(m_showTitle,SIGNAL(stateChanged(int)),this,SLOT(ShowTitleChanged(int)));
   m_title = new QLineEdit(m_plot->GetName());
   grid->addWidget(m_showTitle,row,0);
   grid->addWidget(m_title,row,1);
   connect(m_title,SIGNAL(editingFinished()),this,SLOT(TitleEditFinished()));
   ++row;


   l = new QLabel(tr("X-Axis Label"));
   m_labelX = new QLineEdit(m_plot->GetLabelX());
   grid->addWidget(l,row,0);
   grid->addWidget(m_labelX,row,1);
   connect(m_labelX,SIGNAL(editingFinished()),this,SLOT(LabelXEditFinished()));
   ++row;

   l = new QLabel(tr("Y-Axis Label"));
   m_labelY = new QLineEdit(m_plot->GetLabelY());
   grid->addWidget(l,row,0);
   grid->addWidget(m_labelY,row,1);
   connect(m_labelY,SIGNAL(editingFinished()),this,SLOT(LabelYEditFinished()));
   ++row;

   grid->setSpacing(5);

   //Series
   m_seriesArea = new QVBoxLayout();
   m_seriesArea->setSpacing(5);
   BuildSeriesDisplay();
   auto seriesContainer =  new QWidget();
   auto seriesScroll = new QScrollArea();
   auto seriesBox = new QGroupBox(tr("Series"));
   auto seriesLayout = new QVBoxLayout();
   seriesContainer->setLayout(m_seriesArea);
   seriesScroll->setWidgetResizable(true);
   seriesScroll->setWidget(seriesContainer);
   seriesLayout->addWidget(seriesScroll,1);
   seriesBox->setLayout(seriesLayout);

   //QDoubleValidator* doubleCheck = new QDoubleValidator(this);
   QHBoxLayout* valueLayout;

   double start, end;
   m_plot->GetVisibleX(start, end);
   QGroupBox* displayValuesXBox = new QGroupBox(tr("X-Axis Visible Values"));
   valueLayout = new QHBoxLayout();
   valueLayout->setSpacing(5);

   l = new QLabel(tr("Start"));
   m_startX = new QLineEdit(DoubleToStringComplete(start));
   m_startX->setAlignment(Qt::AlignRight);
   m_startX->setValidator(new QDoubleValidator(this));
   connect(m_startX,SIGNAL(editingFinished()),this,SLOT(StartXEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_startX);

   l = new QLabel(tr("End"));
   m_endX = new QLineEdit(DoubleToStringComplete(end));
   m_endX->setAlignment(Qt::AlignRight);
   m_endX->setValidator(new QDoubleValidator(this));
   connect(m_endX,SIGNAL(editingFinished()),this,SLOT(EndXEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_endX);
   valueLayout->addStretch();

   displayValuesXBox->setLayout(valueLayout);

   m_plot->GetVisibleY(start, end);
   QGroupBox* displayValuesYBox = new QGroupBox(tr("Y-Axis Visible Values"));
   valueLayout = new QHBoxLayout();
   valueLayout->setSpacing(5);

   l = new QLabel(tr("Start"));
   m_startY = new QLineEdit(DoubleToStringComplete(start));
   m_startY->setAlignment(Qt::AlignRight);
   m_startY->setValidator(new QDoubleValidator(this));
   connect(m_startY,SIGNAL(editingFinished()),this,SLOT(StartYEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_startY);

   l = new QLabel(tr("End"));
   m_endY = new QLineEdit(DoubleToStringComplete(end));
   m_endY->setAlignment(Qt::AlignRight);
   m_endY->setValidator(new QDoubleValidator(this));
   connect(m_endY,SIGNAL(editingFinished()),this,SLOT(EndYEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_endY);
   valueLayout->addStretch();

   displayValuesYBox->setLayout(valueLayout);

   m_autoScale = new QCheckBox(tr("Auto-increase range values on new data"));
   m_autoScale->setChecked(m_plot->GetAutoScale());
   connect(m_autoScale,SIGNAL(stateChanged(int)),this,SLOT(AutoScaleChanged(int)));

   m_plot->GetLimitsX(start, end);
   QGroupBox* limitValuesXBox = new QGroupBox(tr("X-Axis Range Values"));
   valueLayout = new QHBoxLayout();
   valueLayout->setSpacing(5);

   l = new QLabel(tr("Min"));
   m_limitStartX = new QLineEdit(DoubleToStringComplete(start));
   m_limitStartX->setAlignment(Qt::AlignRight);
   m_limitStartX->setValidator(new QDoubleValidator(this));
   connect(m_limitStartX,SIGNAL(editingFinished()),this,SLOT(LimitStartXEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_limitStartX);

   l = new QLabel(tr("Max"));
   m_limitEndX = new QLineEdit(DoubleToStringComplete(end));
   m_limitEndX->setAlignment(Qt::AlignRight);
   m_limitEndX->setValidator(new QDoubleValidator(this));
   connect(m_limitEndX,SIGNAL(editingFinished()),this,SLOT(LimitEndXEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_limitEndX);
   valueLayout->addStretch();

   limitValuesXBox->setLayout(valueLayout);

   m_plot->GetLimitsY(start, end);
   QGroupBox* limitValuesYBox = new QGroupBox(tr("Y-Axis Range Values"));
   valueLayout = new QHBoxLayout();
   valueLayout->setSpacing(5);

   l = new QLabel(tr("Min"));
   m_limitStartY = new QLineEdit(DoubleToStringComplete(start));
   m_limitStartY->setAlignment(Qt::AlignRight);
   m_limitStartY->setValidator(new QDoubleValidator(this));
   connect(m_limitStartY,SIGNAL(editingFinished()),this,SLOT(LimitStartYEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_limitStartY);

   l = new QLabel(tr("Max"));
   m_limitEndY = new QLineEdit(DoubleToStringComplete(end));
   m_limitEndY->setAlignment(Qt::AlignRight);
   m_limitEndY->setValidator(new QDoubleValidator(this));
   connect(m_limitEndY,SIGNAL(editingFinished()),this,SLOT(LimitEndYEditFinished()));
   valueLayout->addWidget(l);
   valueLayout->addWidget(m_limitEndY);
   valueLayout->addStretch();

   limitValuesYBox->setLayout(valueLayout);


   //stich it all together
   auto tabs = new QTabWidget();

   auto pageGeneral = new QWidget();
   auto pageGeneralLayout = new QVBoxLayout();
   pageGeneralLayout->addLayout(grid);
   pageGeneralLayout->addWidget(seriesBox,1);
   pageGeneralLayout->setSpacing(0);
   pageGeneral->setLayout(pageGeneralLayout);
   tabs->addTab(pageGeneral,tr("General"));

   auto pageRange = new QWidget();
   auto pageRangeLayout = new QVBoxLayout();
   pageRangeLayout->addWidget(limitValuesXBox);
   pageRangeLayout->addWidget(limitValuesYBox);
   pageRangeLayout->addWidget(m_autoScale);
   pageRangeLayout->addStretch();
   pageRangeLayout->setSpacing(0);
   pageRange->setLayout(pageRangeLayout);
   tabs->addTab(pageRange,tr("Range Values"));

   auto pageVisible = new QWidget();
   auto pageVisibleLayout = new QVBoxLayout();
   pageVisibleLayout->addWidget(displayValuesXBox);
   pageVisibleLayout->addWidget(displayValuesYBox);
   pageVisibleLayout->addStretch();
   pageVisibleLayout->setSpacing(0);
   pageVisible->setLayout(pageVisibleLayout);
   tabs->addTab(pageVisible,tr("Visible Values"));

   setWidget(tabs);

   UpdateWindowTitle();
}


void XYPlotPropertiesView::BuildSeriesDisplay()
{
   std::list<XYSeries*>& list = m_plot->GetSeriesList();
   std::list<XYSeries*>::iterator its;
   for(its=list.begin(); its!=list.end(); ++its)
   {
      m_seriesArea->addWidget(new XYPlotPropertiesSeriesWidget((*its)));
   }
   m_seriesArea->addStretch();
}

void XYPlotPropertiesView::UpdateSeriesDisplay(XYSeries *series)
{
   XYPlotPropertiesSeriesWidget* sw;
   //don't include stretch item
   for(int i=0; i<m_seriesArea->count()-1; ++i)
   {
      sw = (XYPlotPropertiesSeriesWidget*) (m_seriesArea->itemAt(i)->widget());
      if (sw->GetSeries() == series)
      {
         sw->UpdateValueDisplay();
         break;
      }
   }
}

void XYPlotPropertiesView::RemoveSeries(XYSeries *series)
{
   XYPlotPropertiesSeriesWidget* sw;
   //don't include stretch item
   for(int i=0; i<m_seriesArea->count()-1; ++i)
   {
      sw = (XYPlotPropertiesSeriesWidget*) (m_seriesArea->itemAt(i)->widget());
      if (sw->GetSeries() == series)
      {
         m_seriesArea->removeWidget(sw);
         delete sw;
         break;
      }
   }
}

void XYPlotPropertiesView::AddSeries(XYSeries *series)
{   
   //insert before stretch item
   auto seriesWidget = new XYPlotPropertiesSeriesWidget(series);
   m_seriesArea->insertWidget(m_seriesArea->count()-1, seriesWidget);
}

void XYPlotPropertiesView::UpdateWindowTitle()
{
   QString t = tr("%1 Properties").arg(m_plot->GetName());
   this->setWindowTitle(t);
}

void XYPlotPropertiesView::TitleEditFinished()
{
   m_plot->SetName(m_title->text());
   m_plot->Refresh();
}

void XYPlotPropertiesView::LabelYEditFinished()
{
   m_plot->SetLabelY(m_labelY->text());
   m_plot->Refresh();
}

void XYPlotPropertiesView::LabelXEditFinished()
{
   m_plot->SetLabelX(m_labelX->text());
   m_plot->Refresh();
}
void XYPlotPropertiesView::StartXEditFinished()
{
   if (m_startX->text().isEmpty() || !m_plot->SetStartX(m_startX->text().toDouble()))
   {
      double start, end;
      m_plot->GetVisibleX(start, end);
      m_startX->setText(DoubleToStringComplete(start));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();

   }
   else
   {
      m_plot->Refresh();
   }
}
void XYPlotPropertiesView::EndXEditFinished()
{
   if (m_endX->text().isEmpty() || !m_plot->SetEndX(m_endX->text().toDouble()))
   {
      double start, end;
      m_plot->GetVisibleX(start, end);
      m_endX->setText(DoubleToStringComplete(end));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();

   }
   else
   {
      m_plot->Refresh();
   }
}
void XYPlotPropertiesView::StartYEditFinished()
{
   if (m_startY->text().isEmpty() || !m_plot->SetStartY(m_startY->text().toDouble()))
   {
      double start, end;
      m_plot->GetVisibleY(start, end);
      m_startY->setText(DoubleToStringComplete(start));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}
void XYPlotPropertiesView::EndYEditFinished()
{
   if (m_endY->text().isEmpty() || !m_plot->SetEndY(m_endY->text().toDouble()))
   {
      double start, end;
      m_plot->GetVisibleY(start, end);
      m_endY->setText(DoubleToStringComplete(end));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}

void XYPlotPropertiesView::ShowTitleChanged(int)
{
   m_plot->SetShowTitle(m_showTitle->isChecked());
   m_plot->Refresh();
}

void XYPlotPropertiesView::AutoScaleChanged(int)
{
   m_plot->SetAutoScale(m_autoScale->isChecked());
}

void XYPlotPropertiesView::LimitStartXEditFinished()
{
   if (m_limitStartX->text().isEmpty() || !m_plot->SetLimitStartX(m_limitStartX->text().toDouble()))
   {
      double min, max;
      m_plot->GetLimitsX(min, max);
      m_limitStartX->setText(DoubleToStringComplete(min));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}

void XYPlotPropertiesView::LimitEndXEditFinished()
{
   if (m_limitEndX->text().isEmpty() || !m_plot->SetLimitEndX(m_limitEndX->text().toDouble()))
   {
      double min, max;
      m_plot->GetLimitsX(min, max);
      m_limitEndX->setText(DoubleToStringComplete(max));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}

void XYPlotPropertiesView::LimitStartYEditFinished()
{
   if (m_limitStartY->text().isEmpty() || !m_plot->SetLimitStartY(m_limitStartY->text().toDouble()))
   {
      double min, max;
      m_plot->GetLimitsY(min, max);
      m_limitStartY->setText(DoubleToStringComplete(min));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}

void XYPlotPropertiesView::LimitEndYEditFinished()
{
   if (m_limitEndY->text().isEmpty() || !m_plot->SetLimitEndY(m_limitEndY->text().toDouble()))
   {
      double min, max;
      m_plot->GetLimitsY(min, max);
      m_limitEndY->setText(DoubleToStringComplete(max));
      QMessageBox m(this);
      m.setText(tr("Invalid entry."));
      m.exec();
   }
   else
   {
      m_plot->Refresh();
   }
}

void XYPlotPropertiesView::SetStartEndX(double start, double end)
{
   m_startX->setText(DoubleToStringComplete(start));
   m_endX->setText(DoubleToStringComplete(end));
}

void XYPlotPropertiesView::SetStartEndY(double start, double end)
{
   m_startY->setText(DoubleToStringComplete(start));
   m_endY->setText(DoubleToStringComplete(end));
}

void XYPlotPropertiesView::SetLimitStartEndX(double start, double end)
{
   m_limitStartX->setText(DoubleToStringComplete(start));
   m_limitEndX->setText(DoubleToStringComplete(end));
}

void XYPlotPropertiesView::SetLimitStartEndY(double start, double end)
{
   m_limitStartY->setText(DoubleToStringComplete(start));
   m_limitEndY->setText(DoubleToStringComplete(end));
}

//
// Series widgets and fun
XYPlotPropertiesSeriesWidget::SeriesButton::SeriesButton(XYSeries* series) : m_series(series)
{
   this->setFixedSize(50,30);
}
void XYPlotPropertiesSeriesWidget::SeriesButton::paintEvent(QPaintEvent *event)
{
   QPushButton::paintEvent(event);

   QPainter painter(this);
   QRect rect = this->rect();
   int border = this->style()->pixelMetric(QStyle::PM_ButtonMargin,0,this);
   rect.adjust(border,border,border*-1,border*-1);

   QBrush brush(Qt::SolidPattern);
   QColor c = m_series->GetPlot()->GetSeriesAreaBackgroundColor();
   brush.setColor(c);
   painter.setPen(c);
   painter.setBrush(brush);
   painter.drawRect(rect);

   QPen pen(m_series->GetColor(), 3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
   int y = rect.y() + rect.height()/2;
   painter.setPen(pen);
   painter.drawLine(rect.x(),y,rect.x()+rect.width(),y);
}

XYPlotPropertiesSeriesWidget::XYPlotPropertiesSeriesWidget(XYSeries* series) : m_series(series)
{
   m_showOnPlot = new QCheckBox();
   m_showOnPlot->setChecked(series->GetShowOnPlot());
   connect(m_showOnPlot,SIGNAL(toggled(bool)),this,SLOT(OnShowOnCheckBoxToggle(bool)));

   m_button = new SeriesButton(series);
   connect(m_button,SIGNAL(clicked()),this,SLOT(OnSelectColor()));

   m_description = new QLabel(series->GetDescription());

   QPushButton* remove = new QPushButton();
   remove->setIcon(QIcon(":/images/32x32/delete.png"));
   remove->setIconSize(QSize(16,16));
   int border = 2*remove->style()->pixelMetric(QStyle::PM_ButtonMargin,0,remove);
   remove->setFixedSize(16+border,16+border);
   remove->setToolTip(tr("Remove from plot"));
   connect(remove,SIGNAL(clicked()),this,SLOT(OnRemove()));

   QHBoxLayout* l = new QHBoxLayout();
   l->addWidget(m_showOnPlot);
   l->addWidget(m_button);
   l->addWidget(m_description,1);
   l->addWidget(remove);
   l->setSpacing(5);
   this->setLayout(l);
}

void XYPlotPropertiesSeriesWidget::OnSelectColor()
{
   QColor c = QColorDialog::getColor(m_series->GetColor(),this);
   if (c.isValid())
   {
      m_series->SetColor(c);
      m_series->GetPlot()->Refresh();
   }
}

void XYPlotPropertiesSeriesWidget::OnShowOnCheckBoxToggle(bool value)
{
   m_series->SetShowOnPlot(value);
   m_series->GetPlot()->Refresh();
}
void XYPlotPropertiesSeriesWidget::OnRemove()
{
   XYPlot* plot = m_series->GetPlot();
   plot->RemoveSeries(m_series->GetY());
   plot->Refresh();
}

void XYPlotPropertiesSeriesWidget::UpdateValueDisplay()
{
   m_showOnPlot->setChecked(m_series->GetShowOnPlot());
   m_button->update(); //repaint to get new color
   m_description->setText(m_series->GetDescription());
}

}
