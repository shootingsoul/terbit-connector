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

#include "connector-core/WorkspaceDockWidget.h"
#include "SigAnalysisProcessor.h"

namespace terbit
{
class DataClass;

class SignalAnalysisView : public WorkspaceDockWidget
{
   Q_OBJECT
public:
   SignalAnalysisView(SigAnalysisProcessor *proc);
   void dragEnterEvent(QDragEnterEvent *event);
   void dropEvent(QDropEvent *event);

private slots:
   void onNameChanged(DataClass *);
private:
   SigAnalysisProcessor *m_proc;
};

}//terbit
