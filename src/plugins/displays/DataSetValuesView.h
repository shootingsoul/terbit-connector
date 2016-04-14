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

#include <QAbstractTableModel>
#include <QString>
#include "DataSetValues.h"
#include "connector-core/WorkspaceDockWidget.h"

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QScrollBar)

namespace terbit
{

class DataSetValuesViewTableModel;
class BigScrollbar;

class DataSetValuesView: public WorkspaceDockWidget
{
   Q_OBJECT
public:
   DataSetValuesView(DataSetValues* dsv);

   void LoadModel();

   void dragEnterEvent(QDragEnterEvent *event);
   void dropEvent(QDropEvent *event);

protected:
   void resizeEvent(QResizeEvent *event);
private slots:
   void OnModelNameChanged(DataClass*);
   void OnModelNewData();
   void OnModelStructureChanged();

   void OnRefreshData();
   void OnDataTypeChanged(int index);
   void OnColumnsChanged(int index);
   void OnFormatChanged(int index);
   void OnIndexFormatChanged(int index);
   void GotoIndex();
   void OnTableScrolled();

   void OnKeyUp();
   void OnKeyDown();
   void OnKeyPageUp();
   void OnKeyPageDown();
protected:
   void wheelEvent(QWheelEvent *event);

private:
   DataSetValuesView(const DataSetValuesView& o); //disable copy ctor

   void SetupKeyboardShortcuts();
   uint64_t CalculateVisibleRows();
   void UpdateScrollbar();

   DataSetValues* m_bufferValues;
   QTableView* m_table;
   DataSetValuesViewTableModel* m_tableModel;
   QComboBox* m_valueDataType;
   QComboBox* m_valueColumns;
   QComboBox* m_valueFormat;
   QComboBox* m_indexFormat;
   QLineEdit* m_gotoIndex;
   BigScrollbar* m_scroll;
   QPushButton* m_refresh;
   bool m_loading;
};

class DataSetValuesViewTableModel : public QAbstractTableModel
{
   Q_OBJECT
public:
   DataSetValuesViewTableModel(DataSetValues* bufferValues);

   int rowCount(const QModelIndex& parent) const;
   int columnCount(const QModelIndex& parent) const;
   QVariant data(const QModelIndex& index, int role) const;
   QVariant headerData(int section, Qt::Orientation orientation, int role) const;

   void EmitDataChanged();
private:
   DataSetValues* m_bufferValues;
};

}
