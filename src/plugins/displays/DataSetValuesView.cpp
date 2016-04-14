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

#include <QLabel>
#include <QShortcut>
#include <QTableView>
#include <QHeaderView>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QStandardItemModel>
#include "DataSetValuesView.h"
#include "connector-core/DataSet.h"
#include "tools/widgets/BigScrollbar.h"

namespace terbit
{


DataSetValuesView::DataSetValuesView(DataSetValues* dsv) : WorkspaceDockWidget(dsv,dsv->GetName()), m_bufferValues(dsv), m_loading(false)
{
   setAcceptDrops(true);

   SetupKeyboardShortcuts();

   m_table = new QTableView();
   m_tableModel = new DataSetValuesViewTableModel(dsv);
   m_table->setModel(m_tableModel);
   m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   m_scroll = new BigScrollbar(this, Qt::Vertical);

   QFont fixedFont("Monospace");
   fixedFont.setStyleHint(QFont::TypeWriter);
   m_table->setFont(fixedFont);

   m_valueColumns = new QComboBox();
   for (int i = 1; i <=16; ++i)
   {
      m_valueColumns->addItem(QString::number(i),i);
   }

   m_valueFormat = new QComboBox();
   m_valueFormat->addItem(tr("Value"),DataSetValues::DISPLAY_FORMAT_VALUE);
   m_valueFormat->addItem(tr("Hex"),DataSetValues::DISPLAY_FORMAT_HEX);
   m_valueFormat->addItem(tr("Octal"),DataSetValues::DISPLAY_FORMAT_OCT);

   m_valueDataType = new QComboBox();
   m_valueDataType->addItem(tr("uint8"),TERBIT_UINT8);
   m_valueDataType->addItem(tr("uint16"),TERBIT_UINT16);
   m_valueDataType->addItem(tr("uint32"),TERBIT_UINT32);
   m_valueDataType->addItem(tr("uint64"),TERBIT_UINT64);
   m_valueDataType->addItem(tr("int8"),TERBIT_INT8);
   m_valueDataType->addItem(tr("int16"),TERBIT_INT16);
   m_valueDataType->addItem(tr("int32"),TERBIT_INT32);
   m_valueDataType->addItem(tr("int64"),TERBIT_INT64);
   m_valueDataType->addItem(tr("float"),TERBIT_FLOAT);
   m_valueDataType->addItem(tr("double"),TERBIT_DOUBLE);

   m_indexFormat = new QComboBox();
   m_indexFormat->addItem(tr("Value"),DataSetValues::DISPLAY_FORMAT_VALUE);
   m_indexFormat->addItem(tr("Hex"),DataSetValues::DISPLAY_FORMAT_HEX);
   m_indexFormat->addItem(tr("Octal"),DataSetValues::DISPLAY_FORMAT_OCT);

   m_gotoIndex = new QLineEdit();

   m_refresh = new QPushButton(tr("Refresh"));

   QVBoxLayout* l = new QVBoxLayout();

   QHBoxLayout* settingsLayout = new QHBoxLayout();
   settingsLayout->addWidget(new QLabel(tr("Index")));
   settingsLayout->addWidget(new QLabel(tr("Format")));
   settingsLayout->addWidget(m_indexFormat);
   settingsLayout->addWidget(m_gotoIndex);
   QPushButton* btnGotoAddress = new QPushButton(tr("Go To"));
   settingsLayout->addWidget(btnGotoAddress);
   settingsLayout->addStretch();
   l->addLayout(settingsLayout);

   settingsLayout = new QHBoxLayout();
   settingsLayout->addWidget(new QLabel(tr("Data Type")));
   settingsLayout->addWidget(m_valueDataType);
   settingsLayout->addStretch();
   l->addLayout(settingsLayout);

   settingsLayout = new QHBoxLayout();
   settingsLayout->addWidget(new QLabel(tr("Format")));
   settingsLayout->addWidget(m_valueFormat);
   settingsLayout->addWidget(new QLabel(tr("Columns")));
   settingsLayout->addWidget(m_valueColumns);
   settingsLayout->addStretch();
   settingsLayout->addWidget(m_refresh);
   l->addLayout(settingsLayout);

   settingsLayout = new QHBoxLayout();
   settingsLayout->setSpacing(2);
   settingsLayout->setMargin(0);
   settingsLayout->addWidget(m_table,1);
   settingsLayout->addWidget(m_scroll,0);
   l->addLayout(settingsLayout);

   QWidget* wrapper = new QWidget();
   wrapper->setLayout(l);
   setWidget(wrapper); //must use wrapper widget for dockwidget stuffs

   //listen for gui events
   connect(m_valueDataType,SIGNAL(currentIndexChanged(int)), this, SLOT(OnDataTypeChanged(int)));
   connect(m_valueFormat,SIGNAL(currentIndexChanged(int)), this, SLOT(OnFormatChanged(int)));
   connect(m_valueColumns,SIGNAL(currentIndexChanged(int)), this, SLOT(OnColumnsChanged(int)));
   connect(m_indexFormat,SIGNAL(currentIndexChanged(int)), this, SLOT(OnIndexFormatChanged(int)));
   connect(btnGotoAddress,SIGNAL(clicked()),this,SLOT(GotoIndex()));
   connect(m_refresh,SIGNAL(clicked()),this,SLOT(OnRefreshData()));
   connect(m_scroll,SIGNAL(PositionChanged()),this,SLOT(OnTableScrolled()));

   //listen for model events
   connect(m_bufferValues,SIGNAL(ModelNewData()),this, SLOT(OnModelNewData()));
   connect(m_bufferValues,SIGNAL(ModelStructureChanged()), this, SLOT(OnModelStructureChanged()));
   connect(m_bufferValues,SIGNAL(NameChanged(DataClass*)),this, SLOT(OnModelNameChanged(DataClass*)));

   //get gui in sync with settings . . . simulate structure change
   OnModelStructureChanged();
   m_table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
}

void DataSetValuesView::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
   {
      event->acceptProposedAction();
   }
}

void DataSetValuesView::dropEvent(QDropEvent *event)
{
   QStandardItemModel model;
   model.dropMimeData(event->mimeData(), Qt::CopyAction, 0,0, QModelIndex());

   int numRows = model.rowCount();
   for (int row = 0; row < numRows; ++row)
   {
      QModelIndex index = model.index(row, 0);
      DataClassAutoId_t id = model.data(index, Qt::UserRole).toUInt();
      m_bufferValues->ApplyInput(id);
   }
   event->acceptProposedAction();

}

void DataSetValuesView::OnRefreshData()
{
   m_bufferValues->RefreshData();
}

void DataSetValuesView::OnDataTypeChanged(int index)
{
   index;
   if (m_loading == false)
   {
      TerbitDataType valueType = (TerbitDataType)m_valueDataType->currentData().toInt();
      m_bufferValues->SetDataType(valueType);
   }
}

void DataSetValuesView::OnColumnsChanged(int index)
{
   index;
   if (m_loading == false)
   {
      int valueColumns = m_valueColumns->currentData().toInt();
      m_bufferValues->SetColumns(valueColumns);
      m_table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
   }
}

void DataSetValuesView::OnFormatChanged(int index)
{
   index;
   if (m_loading == false)
   {
      DataSetValues::DisplayFormat valueFormat =  (DataSetValues::DisplayFormat)m_valueFormat->currentData().toInt();
      m_bufferValues->SetFormat(valueFormat);
   }
}

void DataSetValuesView::OnIndexFormatChanged(int index)
{
   index;
   if (m_loading == false)
   {
      DataSetValues::DisplayFormat indexFormat =  (DataSetValues::DisplayFormat)m_indexFormat->currentData().toInt();
      m_bufferValues->SetIndexFormat(indexFormat);
      m_table->horizontalHeader()->resizeSections(QHeaderView::Stretch);
   }
}

void DataSetValuesView::GotoIndex()
{
   uint64_t index;
   if (m_bufferValues->StringToValue(index, m_gotoIndex->text(), m_bufferValues->GetIndexFormat()))
   {
      if (!m_bufferValues->MoveToIndex(index))
      {
         QMessageBox m(this);
         m.setText(tr("Index out of range."));
         m.exec();
      }
   }
   else
   {
      QMessageBox m(this);
      m.setText(tr("Invalid index."));
      m.exec();
   }
}

void DataSetValuesView::OnModelNameChanged(DataClass *)
{
   setWindowTitle(m_bufferValues->GetName());
}

void DataSetValuesView::OnModelNewData()
{
   //keep table and scrollbar in sync with new data
   if (m_scroll->GetPosition() != m_bufferValues->GetVisibleRowStart())
   {
      m_scroll->SetPosition(m_bufferValues->GetVisibleRowStart());
   }
   emit m_tableModel->EmitDataChanged();
}

void DataSetValuesView::OnModelStructureChanged()
{
   //sync gui to model settings . . .
   m_loading = true;
   m_valueColumns->setCurrentIndex(m_valueColumns->findData((int)m_bufferValues->GetColumns()));
   m_valueDataType->setCurrentIndex(m_valueDataType->findData(m_bufferValues->GetDataType()));
   m_valueFormat->setCurrentIndex(m_valueFormat->findData(m_bufferValues->GetFormat()));
   m_indexFormat->setCurrentIndex(m_valueFormat->findData(m_bufferValues->GetIndexFormat()));
   m_refresh->setVisible(m_bufferValues->CanRefreshData());
   UpdateScrollbar();
   m_loading = false;

   //always force tableview to rebuild
   emit m_tableModel->layoutChanged();
   emit m_tableModel->headerDataChanged(Qt::Vertical,0,m_bufferValues->GetVisibleRowCount()-1); //also update row index/address
}
void DataSetValuesView::UpdateScrollbar()
{
   uint64_t rows = m_bufferValues->GetVisibleRowCount();
   m_scroll->SetPageStep(rows);
   m_scroll->SetSingleStep(1);

   //don't include page/visible rows in the scrollbar count
   if (rows >= m_bufferValues->GetRows())
   {
      rows = 1;
   }
   else
   {
      rows = m_bufferValues->GetRows() - rows + 1;
   }
   m_scroll->SetScrollbar(0,rows,m_bufferValues->GetVisibleRowStart());
}


uint64_t DataSetValuesView::CalculateVisibleRows()
{
   int rowHeight = m_table->rowHeight(0);
   if (rowHeight == 0)
   {
      rowHeight = 30; // fancy a guess, don't have an actual row yet . . .
   }
   uint64_t rows = m_table->height() / rowHeight;
   if (rows != 0)
   {
      --rows; //remove space for header
   }
   return rows;
}

void DataSetValuesView::OnTableScrolled()
{
   if (!m_loading)
   {
      m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
   }
}

void DataSetValuesView::wheelEvent(QWheelEvent *event)
{
   if (event->delta() > 0)
   {
      m_scroll->MovePreviousStep();
   }
   else
   {
      m_scroll->MoveNextStep();
   }

   m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
}

void DataSetValuesView::OnKeyUp()
{
   //keys only work if in the table . . . see if we need to scroll the data in underlying buffer
   QModelIndex i = m_table->currentIndex();

   if (i.isValid() && m_table->hasFocus())
   {
      if (i.row() == 0)
      {
         m_scroll->MovePreviousStep();
         m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
      }
      else
      {
         QModelIndex nextIndex = m_table->model()->index(i.row()-1, i.column());
         m_table->setCurrentIndex(nextIndex);
      }
   }
}

void DataSetValuesView::OnKeyDown()
{
   //keys only work if in the table . . . see if we need to scroll the data in underlying buffer
   QModelIndex i = m_table->currentIndex();

   if (i.isValid() && m_table->hasFocus())
   {
      if (i.row() == m_tableModel->rowCount(i) - 1)
      {
         m_scroll->MoveNextStep();
         m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
      }
      else
      {
         QModelIndex nextIndex = m_table->model()->index(i.row()+1, i.column());
         m_table->setCurrentIndex(nextIndex);
      }
   }
}

void DataSetValuesView::OnKeyPageUp()
{
   m_scroll->MovePreviousPage();
   m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
}
void DataSetValuesView::OnKeyPageDown()
{
   m_scroll->MoveNextPage();
   m_bufferValues->UpdateVisibleRows(m_scroll->GetPosition(), m_scroll->GetPageStep());
}

void DataSetValuesView::SetupKeyboardShortcuts()
{
   QShortcut* s;

   s = new QShortcut(QKeySequence(Qt::Key_Up), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnKeyUp()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_Down), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnKeyDown()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageUp), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnKeyPageUp()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

   s = new QShortcut(QKeySequence(Qt::Key_PageDown), this);
   connect(s, SIGNAL(activated()), this, SLOT(OnKeyPageDown()));
   s->setContext(Qt::WidgetWithChildrenShortcut);

}

void DataSetValuesView::resizeEvent(QResizeEvent *event)
{
   //keep visible rows in sync with size . . .
   uint64_t visibleRows = CalculateVisibleRows();
   m_bufferValues->UpdateVisibleRows(m_bufferValues->GetVisibleRowStart(), visibleRows);
   UpdateScrollbar();

   //always force tableview to rebuild
   emit m_tableModel->layoutChanged();
}


DataSetValuesViewTableModel::DataSetValuesViewTableModel(DataSetValues *bufferValues) : m_bufferValues(bufferValues)
{

}

int DataSetValuesViewTableModel::rowCount(const QModelIndex &parent) const
{
   //include extra row to indicate there are more rows below . . .
   if (m_bufferValues->GetVisibleRowStart()+ m_bufferValues->GetVisibleRowCount() < m_bufferValues->GetRows())
   {
      return (int)m_bufferValues->GetVisibleRowCount()+1;
   }
   else
   {
      return (int)m_bufferValues->GetVisibleRowCount();
   }
}

int DataSetValuesViewTableModel::columnCount(const QModelIndex &parent) const
{
   return (int)m_bufferValues->GetColumns();
}

QVariant DataSetValuesViewTableModel::data(const QModelIndex& index, int role) const
{
   if(role == Qt::DisplayRole)
   {
      uint64_t row = m_bufferValues->GetVisibleRowStart() + index.row();
      uint64_t col = index.column();
      return m_bufferValues->GetDisplayValue(row, col);
   }
   else if (role == Qt::TextAlignmentRole)
   {
      return Qt::AlignRight;
   }

   return QVariant::Invalid;
}

QVariant DataSetValuesViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

   if(role == Qt::DisplayRole)
   {
      if(orientation == Qt::Horizontal)
      {
         return m_bufferValues->GetDisplayColumnIndex(section);
      }
      else if(orientation == Qt::Vertical)
      {
         return m_bufferValues->GetDisplayRowIndex(section + m_bufferValues->GetVisibleRowStart());
      }
   }
   else if (role == Qt::TextAlignmentRole && orientation == Qt::Vertical)
   {
      return Qt::AlignRight;
   }

   return QVariant::Invalid;
}

void DataSetValuesViewTableModel::EmitDataChanged()
{   
   QModelIndex start = QAbstractItemModel::createIndex(0,0);
   int lastRow = rowCount(start);
   int lastCol = columnCount(start);
   emit dataChanged(start,QAbstractItemModel::createIndex(lastRow, lastCol));
   emit headerDataChanged(Qt::Vertical,0,lastRow); //also update row index/address
}


}

