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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QHeaderView>
#include "PluginsView.h"
#include "BuildInfoCore.h"
#include "ResourceManager.h"
#include "tools/Tools.h"

namespace terbit
{

PluginsView::PluginsView(QWidget *parent, const PluginList& plugins, DataClassManager& manager) : QDialog(parent)
{
   setWindowTitle(_STR_PRODUCT_NAME);

   QVBoxLayout* l = new QVBoxLayout();
   QHBoxLayout* layoutProduct = new QHBoxLayout();
   QVBoxLayout* layoutVersion = new QVBoxLayout();

   QPixmap pix(ResourceManager::GetLogo());
   QLabel* logo = new QLabel();
   logo->setPixmap(pix);
   logo->setFixedSize(pix.size());


   layoutProduct->addWidget(logo);
   layoutVersion->addStretch();
   layoutVersion->addWidget(new QLabel(tr("Version: %1 %2").arg(_STR_PRODUCT_VERSION).arg(CompilerOptions())));
   QString buildId = BUILD_ID_STR;
   buildId = buildId.left(10);
   layoutVersion->addWidget(new QLabel(tr("Build: %1").arg(buildId)));
   layoutProduct->addLayout(layoutVersion);
   layoutProduct->addStretch();

   l->addSpacing(5);
   l->addLayout(layoutProduct);
   l->addSpacing(30);


   //Plugins
   QTableWidget* table = new QTableWidget();
   table->setColumnCount(5);
   QStringList headers;
   headers << tr("Namespace") << tr("Description") << tr("Version") << tr("File") << tr("Directory");
   table->setHorizontalHeaderLabels(headers);
   table->verticalHeader()->setVisible(false);

   table->setRowCount(plugins.size());
   int row = 0;
   for(PluginList::const_iterator it = plugins.begin(); it != plugins.end(); ++it)
   {
      Plugin* p = *it;

      QTableWidgetItem* cell;

      cell = new QTableWidgetItem(p->GetNamespace());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      table->setItem(row,0, cell);

      cell = new QTableWidgetItem(p->GetDescription());
      cell->setTextAlignment(Qt::AlignVCenter);
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      table->setItem(row,1, cell);

      cell = new QTableWidgetItem(p->GetVersion());
      cell->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      table->setItem(row,2, cell);

      cell = new QTableWidgetItem(p->GetFileName());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      table->setItem(row,3, cell);

      cell = new QTableWidgetItem(p->GetDirectory());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      table->setItem(row,4, cell);
      ++row;
   }
   table->resizeColumnToContents(0);
   table->resizeColumnToContents(2);

   //types
   QTableWidget* typesTable = new QTableWidget();
   typesTable->setColumnCount(3);
   headers.clear();
   headers << tr("Name") << tr("Type") << tr("Description");
   typesTable->setHorizontalHeaderLabels(headers);
   typesTable->verticalHeader()->setVisible(false);

   typesTable->setRowCount(manager.GetTypes().size());
   row = 0;
   for(auto& it : manager.GetTypes())
   {
      DataClassType* t = it.second;

      QTableWidgetItem* cell;

      cell = new QTableWidgetItem(t->GetDisplayName());
      cell->setIcon(t->GetGraphics32());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      typesTable->setItem(row,0, cell);

      cell = new QTableWidgetItem(t->GetFullTypeName());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      typesTable->setItem(row,1, cell);

      cell = new QTableWidgetItem(t->GetDescription());
      cell->setFlags(cell->flags() ^ Qt::ItemIsEditable); //disable edits
      typesTable->setItem(row,2, cell);

      ++row;
   }
   typesTable->resizeColumnToContents(0);
   typesTable->resizeColumnToContents(1);

   //-----------------
   //Add plugins/types to the layout
   QFont pluginFont;
   pluginFont.setBold(true);
   QLabel* pluginLabel = new QLabel(tr("Plugins"));
   pluginLabel->setFont(pluginFont);
   l->addWidget(pluginLabel);
   l->addWidget(table, 2);


   QLabel* typesLabel = new QLabel(tr("Types"));
   typesLabel->setFont(pluginFont);
   l->addWidget(typesLabel);
   l->addWidget(typesTable, 4);

   QDialogButtonBox* b = new QDialogButtonBox(this);
   b->addButton(QDialogButtonBox::Ok);
   l->addWidget(b);
   connect(b, SIGNAL(accepted()), this, SLOT(accept()));

   setLayout(l);

   setMinimumWidth(600);
}


}

