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
#include "AboutView.h"
#include "BuildInfoCore.h"
#include "ResourceManager.h"
#include "tools/Tools.h"

namespace terbit
{

AboutView::AboutView(QWidget *parent) : QDialog(parent)
{
   setWindowTitle(_STR_PRODUCT_NAME);

   QVBoxLayout* l = new QVBoxLayout();
   QVBoxLayout* layoutProduct = new QVBoxLayout();
   QHBoxLayout* layoutProductCenter = new QHBoxLayout();

   QPixmap pix(ResourceManager::GetLogo());
   QLabel* logo = new QLabel();
   logo->setPixmap(pix);
   logo->setFixedSize(pix.size());


   layoutProduct->addWidget(logo);
   QLabel* lbl = new QLabel(tr("Version: %1 %2").arg(_STR_PRODUCT_VERSION).arg(CompilerOptions()));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   layoutProduct->addWidget(lbl);
   QString buildId = BUILD_ID_STR;
   buildId = buildId.left(10);
   lbl = new QLabel(tr("Build: %1").arg(buildId));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   layoutProduct->addWidget(lbl);
   layoutProduct->addSpacing(5);
   lbl = new QLabel(tr("Compiler: %1").arg(CompilerInfo()));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   layoutProduct->addWidget(lbl);
   lbl = new QLabel(tr("Compiled Qt: %1").arg(QT_VERSION_STR));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   layoutProduct->addWidget(lbl);
   lbl = new QLabel(tr("Runtime Qt: %1").arg(qVersion()));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   layoutProduct->addWidget(lbl);

   //strech sides to center product info
   layoutProductCenter->addStretch(1);
   layoutProductCenter->addLayout(layoutProduct);
   layoutProductCenter->addStretch(1);

   l->addSpacing(5);
   l->addLayout(layoutProductCenter);
   l->addSpacing(15);

   QLabel* terbitLink = new QLabel();
   //terbitLink->setTextInteractionFlags(Qt::TextSelectableByMouse);
   terbitLink->setOpenExternalLinks(true);
   terbitLink->setText("<a href=\"https://github.com/shootingsoul/terbit-connector\">github terbit-connector</a>");
   l->addWidget(terbitLink,0,Qt::AlignCenter);

   lbl = new QLabel(QString(_STR_LEGAL_COPYRIGHT));
   lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   l->addWidget(lbl,0,Qt::AlignCenter);

   lbl = new QLabel(QString(_STR_LEGAL_TRADE_1));
   //lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
   lbl->setOpenExternalLinks(true);
   lbl->setText("<a href=\"https://www.apache.org/licenses/LICENSE-2.0.html\">Apache License 2.0</a>");
   l->addWidget(lbl,0,Qt::AlignCenter);


   QDialogButtonBox* b = new QDialogButtonBox(this);
   b->addButton(QDialogButtonBox::Ok);
   l->addWidget(b);
   connect(b, SIGNAL(accepted()), this, SLOT(accept()));

   setLayout(l);
}


}
