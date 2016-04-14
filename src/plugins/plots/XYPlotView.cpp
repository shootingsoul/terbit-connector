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
#include <QtGui>
#include <QPoint>
#include <QWheelEvent>
#include "XYPlotView.h"
#include "XYPlot.h"
#include "XYPlotRenderer.h"
#include "tools/widgets/ZoomScrollbar.h"
#include "connector-core/Workspace.h"

namespace terbit
{

////////////////////////////////////////////////
//XYPlotViewArea

XYPlotViewArea::XYPlotViewArea(QWidget* parent, XYPlot* plot)
   : QWidget(parent), m_state(STATE_IDLE), m_plot(plot), m_backgroundAndAxis(NULL), m_rebuildAxis(false)
{
   this->setAttribute(Qt::WA_DeleteOnClose);
   setAutoFillBackground(false);
   setAttribute(Qt::WA_OpaquePaintEvent, true);
   setAttribute(Qt::WA_NoSystemBackground, true);
   setMouseTracking(true);
   this->setFocusPolicy(Qt::NoFocus);

   BuildAxis(this->width(), this->height());
}
XYPlotViewArea::~XYPlotViewArea()
{
   delete m_backgroundAndAxis;
}

void XYPlotViewArea::RebuildAxis()
{
   m_rebuildAxis = true;
}

void XYPlotViewArea::paintEvent(QPaintEvent *)
{
   if (m_rebuildAxis)
   {
      m_rebuildAxis = false;
      BuildAxis(this->width(), this->height());
   }

   QPainter painter;
   painter.begin(this);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.drawImage(0,0,*m_backgroundAndAxis);
   m_plot->GetRenderer()->RenderSeriesArea(&painter);
   QPoint mousePos = this->mapFromGlobal(QCursor::pos());
   m_plot->GetRenderer()->RenderOverlay(&painter, mousePos.x(), mousePos.y());
   painter.end();
}

void XYPlotViewArea::mousePressEvent(QMouseEvent *event)
{
   if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
   {
      event->ignore();
      return;
   }

   QPoint pos = event->pos();
   if (m_plot->GetRenderer()->SeriesAreaContainsPoint(pos.x(), pos.y()))
   {
      this->setCursor(Qt::ClosedHandCursor);
      m_state = STATE_PANNING;
      m_lastMousePos = pos;
   }
   event->accept();
}

void XYPlotViewArea::mouseMoveEvent(QMouseEvent *event)
{

   if (m_state == STATE_PANNING)
   {
      QPoint pos = event->pos();
      if (m_plot->GetRenderer()->SeriesAreaContainsPoint(pos.x(), pos.y()))
      {
         //pan it in data units so it looks super smooth
         //reverse Y logic here
         double scaleX, scaleY;
         m_plot->GetRenderer()->GetScales(scaleX, scaleY);
         if (m_plot->Pan((m_lastMousePos.x()-pos.x())*scaleX,(pos.y()-m_lastMousePos.y())*scaleY))
         {
            m_plot->Refresh();
         }
         m_lastMousePos = pos;
         //pan updates display
      }
      else
      {
         //outsize series area, no longer panning
         this->setCursor(Qt::ArrowCursor);
         m_state = STATE_IDLE;
         this->update(); //to update mouse overlay display (remove it)
      }
   }
   else
   {
      this->update(); //to update mouse overlay display (possibly remove it)
   }
}
void XYPlotViewArea::mouseReleaseEvent(QMouseEvent *)
{
   if (m_state == STATE_PANNING)
   {
      this->setCursor(Qt::ArrowCursor);
      m_state = STATE_IDLE;
   }
}

void XYPlotViewArea::resizeEvent(QResizeEvent *event)
{
   BuildAxis(event->size().width(), event->size().height());
   QWidget::resizeEvent(event);
}

void XYPlotViewArea::BuildAxis(int plotWidth, int plotHeight)
{
   //use same format as raster engine . . . so fast as can be when drawing the image
   if (!m_backgroundAndAxis || m_backgroundAndAxis->width() != plotWidth || m_backgroundAndAxis->height() != plotHeight)
   {
      delete m_backgroundAndAxis;
      m_backgroundAndAxis = new QImage(plotWidth, plotHeight,QImage::Format_ARGB32_Premultiplied);
   }

   QPainter painter;
   painter.begin(m_backgroundAndAxis);
   painter.setRenderHint(QPainter::Antialiasing);
   m_plot->GetRenderer()->RenderBackgroundAndAxis(&painter, plotWidth, plotHeight);
   painter.end();
}


void XYPlotViewArea::wheelEvent(QWheelEvent *event)
{
   double dataX, dataY;
   QPoint pos = event->pos();

   if (m_plot->GetRenderer()->ScreenToDataPoint(pos.x(), pos.y(), dataX, dataY))
   {
      if (event->delta() > 0)
      {
         //up/forward
         if ((event->modifiers() & Qt::CTRL) == Qt::CTRL)
         {
            //zooming
            m_plot->ZoomIn(dataX, dataY);
         }
         else if ((event->modifiers() & Qt::ALT) == Qt::ALT)
         {
            //axis zoom
            if ((event->modifiers() & Qt::SHIFT) == Qt::SHIFT)
            {
               m_plot->ZoomInX(dataX);
            }
            else
            {
               m_plot->ZoomInY(dataY);
            }
         }
         else
         {
            //scrolling
            if ((event->modifiers() & Qt::SHIFT) == Qt::SHIFT)
            {
               m_plot->StepNextX();
            }
            else
            {
               m_plot->StepNextY();
            }
         }
      }
      else
      {
         //down/backwards
         if ((event->modifiers() & Qt::CTRL) == Qt::CTRL)
         {
            //zooming
            m_plot->ZoomOut(dataX, dataY);
         }
         else if ((event->modifiers() & Qt::ALT) == Qt::ALT)
         {
            //axis zoom
            if ((event->modifiers() & Qt::SHIFT) == Qt::SHIFT)
            {
               m_plot->ZoomOutX(dataX);
            }
            else
            {
               m_plot->ZoomOutY(dataY);
            }
         }
         else
         {
            //scrolling
            if ((event->modifiers() & Qt::SHIFT) == Qt::SHIFT)
            {
               m_plot->StepPreviousX();
            }
            else
            {
               m_plot->StepPreviousY();
            }
         }
      }
      m_plot->Refresh();
      parentWidget()->setFocus(); //ensure parent has focus, now
   }   
}

////////////////////////////////////////////////
//XYPlotView


XYPlotView::XYPlotView(XYPlot* plot) : WorkspaceDockWidget(plot, plot->GetName()), m_plot(plot)
{
   this->setAcceptDrops(true);
   this->setFocusPolicy(Qt::StrongFocus); //keep focus here for keyboard shortcuts
   m_viewArea = new XYPlotViewArea(this, plot);

   SetupKeyboardShortcuts();

   m_scrollX = new ZoomScrollbar(this);
   m_scrollY = new ZoomScrollbar(this, Qt::Vertical);

   connect( m_scrollX, SIGNAL( ValueChanged() ), this, SLOT( OnScrollbarX()) );
   connect( m_scrollY, SIGNAL( ValueChanged() ), this, SLOT( OnScrollbarY()) );

   QGridLayout* grid = new QGridLayout();
   grid->addWidget(m_viewArea,0,0);
   grid->addWidget(m_scrollX,1,0);
   grid->addWidget(m_scrollY,0,1);

   grid->setRowStretch(0,1);
   grid->setColumnStretch(0,1);
   grid->setSpacing(0);
   grid->setMargin(0);

   QWidget* container = new QWidget();
   container->setLayout(grid);

   setWidget(container);
}

void XYPlotView::OnScrollbarX()
{
   m_plot->UpdatedVisibleX();
   m_plot->Refresh();
}
void XYPlotView::OnScrollbarY()
{
   m_plot->UpdatedVisibleY();
   m_plot->Refresh();
}

void XYPlotView::Replot()
{
   //update is thread-safe (pushes event to main gui loop for processing), repaint calls paintEvent directly
   //had issues calling repaint from drag/drop (other xy plot event) so call update even if in gui thread
   m_viewArea->update();
}

void XYPlotView::RebuildAxis()
{
   m_viewArea->RebuildAxis();
}

void XYPlotView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
   {
      event->acceptProposedAction();
   }
}

void XYPlotView::dropEvent(QDropEvent *event)
{
   QStandardItemModel model;
   model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex());

   int numRows = model.rowCount();
   for (int row = 0; row < numRows; ++row)
   {
      QModelIndex index = model.index(row, 0);
      DataClassAutoId_t id = model.data(index, Qt::UserRole).toUInt();
      m_plot->ApplyInput(id);
   }
   event->acceptProposedAction();
}

void XYPlotView::contextMenuEvent(QContextMenuEvent *event)
{
   QMenu menu(this);
   QMenu *dsMenu;
   QAction* a;
   DataSetAction* dsa;
   XYSeriesAction* xysa;

   bool first = true;

   //data sets add/remove
   for(auto& dsi :  m_plot->GetWorkspace()->GetDataClassManager().GetInstances())
   {
      DataClass* dc = dsi.second;
      if (dc->IsDataSet() && dc->GetPublicScope())
      {

         DataSet* buf = static_cast<DataSet*>(dc);

         if (first)
         {
            dsMenu = menu.addMenu(tr("Data Sets"));
            dsMenu->setIcon(QIcon(":/images/32x32/database_green.png"));
            first = false;
         }

         dsa = new DataSetAction(buf, dsMenu);
         dsa->setText(buf->GetName()); //keep original name
         bool onPlot = m_plot->IsDataSetOnPlot(buf->GetAutoId());

         if (onPlot)
         {
            dsa->setIcon(QIcon(":/images/32x32/delete.png"));
            connect(dsa, SIGNAL(triggered()),this, SLOT(OnContextDataSourceRemove()));
         }
         else
         {
            dsa->setIcon(QIcon(":/images/32x32/add.png"));
            connect(dsa, SIGNAL(triggered()),this, SLOT(OnContextDataSourceAdd()));
         }
         dsMenu->addAction(dsa);
      }
   }

   //series on plots show/hide (check marks)
   std::list<XYSeries*>& list = m_plot->GetSeriesList();
   std::list<XYSeries*>::iterator its;
   if (list.size() > 0)
   {
      menu.addSeparator();
      for(its=list.begin(); its!=list.end(); ++its)
      {
         xysa = new XYSeriesAction((*its), &menu);
         xysa->setText((*its)->GetDescription());
         xysa->setCheckable(true);
         if ((*its)->GetShowOnPlot())
         {
            xysa->setChecked(true);
            connect(xysa, SIGNAL(triggered()),this, SLOT(OnContextDataSourceHide()));
         }
         else
         {
            xysa->setChecked(false);
            connect(xysa, SIGNAL(triggered()),this, SLOT(OnContextDataSourceShow()));
         }
         menu.addAction(xysa);
      }

      menu.addSeparator();
      a = menu.addAction(tr("Size to Fit"));
      connect(a, SIGNAL(triggered()),this, SLOT(OnContextSizeToFit()));
   }

   //random plot actions
   menu.addSeparator();
   a = menu.addAction(tr("X Axis Grid Lines"));
   connect(a, SIGNAL(triggered()),this, SLOT(OnContextGridLinesX()));
   a->setCheckable(true);
   a->setChecked(m_plot->GetGridLinesX());
   a = menu.addAction(tr("Y Axis Grid Lines"));
   connect(a, SIGNAL(triggered()),this, SLOT(OnContextGridLinesY()));
   a->setCheckable(true);
   a->setChecked(m_plot->GetGridLinesY());
   menu.addSeparator();
   a = menu.addAction(tr("Properties"));
   connect(a, SIGNAL(triggered()),this, SLOT(OnContextProperties()));

   menu.exec(event->globalPos());
}

void XYPlotView::OnContextDataSourceHide()
{
   ((XYSeriesAction*)sender())->series->SetShowOnPlot(false);
   m_plot->Refresh();
}
void XYPlotView::OnContextDataSourceShow()
{
   ((XYSeriesAction*)sender())->series->SetShowOnPlot(true);
   m_plot->Refresh();
}
void XYPlotView::OnContextDataSourceRemove()
{
   DataSet* s = ((DataSetAction*)sender())->dataSet;
   m_plot->RemoveSeries(s);
   m_plot->Refresh();
}
void XYPlotView::OnContextDataSourceAdd()
{
   DataSet* s = ((DataSetAction*)sender())->dataSet;
   m_plot->ApplyInputDataClass(s);
   m_plot->Refresh();
}
void XYPlotView::OnContextSizeToFit()
{
   m_plot->SizeToFit();
   m_plot->Refresh();
}
void XYPlotView::OnContextGridLinesX()
{
   m_plot->SetGridLinesX(!m_plot->GetGridLinesX());
   m_plot->Refresh();
}
void XYPlotView::OnContextGridLinesY()
{
   m_plot->SetGridLinesY(!m_plot->GetGridLinesY());
   m_plot->Refresh();
}
void XYPlotView::OnContextProperties()
{
   m_plot->ShowPropertiesView();
}

void XYPlotView::SetupKeyboardShortcuts()
{
   QShortcut *s;

   s = new QShortcut(QKeySequence(Qt::Key_Left), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Right), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Up), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Down), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageUp), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(PageNextY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageDown), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(PagePreviousY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageUp + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(PageNextX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageDown + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(PagePreviousX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Home), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(MoveFirstX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_End), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(MoveLastX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Home + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(MoveFirstY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_End + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(MoveLastY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Left + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousStartHandleX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Right + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextEndHandleX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Up + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextEndHandleY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Down + Qt::CTRL), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousStartHandleY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Left + Qt::ALT), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousEndHandleX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Right + Qt::ALT), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextStartHandleX()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Up + Qt::ALT), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepNextStartHandleY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Down + Qt::ALT), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(StepPreviousEndHandleY()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Plus), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(ZoomIn()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Minus), this);
   connect(s, SIGNAL(activated()), m_plot, SLOT(ZoomOut()));
   s->setContext(Qt::WidgetWithChildrenShortcut);
}

}

