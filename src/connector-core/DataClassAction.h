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

//! This class simply extends QAction by adding a few data parameters that
//! we use often - i.e. the ID of a Plugin, device, data set, dataplot, etc.
//! This helps us build menus that call system functions on specific instances
//! of those objects.
//!

#pragma once
#include <QAction>
#include "DataClass.h"

QT_BEGIN_NAMESPACE
class QString;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace terbit
{

class DataClassAction : public QAction
{
public:
    // TODO: Implement the rest of the QAction constructors?
   DataClassAction(const QString & text, QObject * parent) : QAction(text, parent), m_PIId(INVALID_ID_VAL), m_DPId(INVALID_ID_VAL), m_DSId(INVALID_ID_VAL), m_DDId(INVALID_ID_VAL)  {}
   DataClassAutoId_t m_PIId;
   DataClassAutoId_t m_DPId;
   DataClassAutoId_t m_DSId;
   DataClassAutoId_t m_DDId;
   QTreeWidgetItem* m_pTreeItem;
};


}// terbit
