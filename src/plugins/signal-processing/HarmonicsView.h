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
#include "SigAnalysisProcessor.h"
#include <QWidget>
#include <QLabel>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)


namespace terbit
{

class HarmonicsRow;

class HarmonicsView : public QWidget
{
   Q_OBJECT
public:
   explicit HarmonicsView(SigAnalysisProcessor* proc);

signals:

public slots:
   void OnDvcUpdate();

private:
   HarmonicsView(const HarmonicsView& o); //disable copy ctor
   void CreateGridStructure();
   QLabel* BuildHeaderLabel(const QString& text, const QString& toolTip);
   QLabel* BuildValueLabel();
   void UpdateGridValues();

   SigAnalysisProcessor* m_dvc = NULL;
   QGridLayout *m_layout = NULL;
   QHBoxLayout* m_mainLayout = NULL;
};

}//terbit
