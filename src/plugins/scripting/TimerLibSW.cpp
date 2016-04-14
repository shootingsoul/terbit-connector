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
#include "TimerLibSW.h"
#include "tools/Tools.h"
#include "ScriptProcessor.h"
#include "connector-core/Workspace.h"
#include "connector-core/ScriptDocumentation.h"
#include <QCoreApplication>

namespace terbit
{


TimerLibSW::TimerLibSW(Workspace *w, ScriptProcessor *ide, QJSEngine *engine) : m_workspace(w), m_ide(ide), m_se(engine)
{
   m_timer = new QTimer(this);
   connect(m_timer, SIGNAL(timeout()), this, SIGNAL(TimerExpired()));
}

TimerLibSW::~TimerLibSW()
{
}


ScriptDocumentation* BuildScriptDocumentationTimerLibSW()
{
   ScriptDocumentation* d = new ScriptDocumentation();

   d->SetName(TimerLibSW::tr("TimerLibSW", "Timer Lib"));
   d->SetSummary(TimerLibSW::tr("TimerLibSW", "A TimerLibSW proivdes a wrapper to a QTimer object, allowing callbacks into Javascript code."));

   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("CreateTimer"),"terbit.CreateTimer();",TimerLibSW::tr("Create an instance of a TimerLibSW object, which provides QTimer functionality.")));

   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("TimerExpired"),"TimerExpired.connect(OnTimerExpire);",TimerLibSW::tr("Connect function to the timer expiration event")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("Start(ms)"), "Start(ms);",TimerLibSW::tr("Starts a timer that will expire after the specified number of milliseconds.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("Start(void)"), "Start();",TimerLibSW::tr("Starts a timer that will expire after the number of milliseconds specified by SetInterval().")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("Stop"), "Stop();",TimerLibSW::tr("Stops the timer.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("Interval"), "Interval();",TimerLibSW::tr("Returns the number of milliseconds this timer is configured for.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("IsActive"), "IsActive();",TimerLibSW::tr("Returns true if the timer is running.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("IsSingleShot"), "IsSingleShot();", TimerLibSW::tr("Returns true if the timer is configured as \"single shot\" as opposed to \"periodic\".")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("RemainingTime"), "RemainingTime();",TimerLibSW::tr("Returns the time remaining (in milliseconds) before the timer expires.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("SetInterval"), "SetInterval(ms);",TimerLibSW::tr("Sets the timer timeout period in milliseconds.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("SetSingleShot"), "SetSingleShot(true);",TimerLibSW::tr("Configures the timer as single shot(true) or periodic (false)")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("SetTimerType"), "SetTimerType(type);",TimerLibSW::tr("Controls timer precision. 0 = precise. 1 = coarse. 2 = very coarse.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("TimerId"), "TimerId();",TimerLibSW::tr("Returns the Qt framework ID for the timer.")));
   d->AddScriptlet(new Scriptlet(TimerLibSW::tr("TimerType"), "TimerType();",TimerLibSW::tr("Returs the type of timer. 0 = precise. 1 = coarse. 2 = very coarse.")));

   ScriptDocumentation* tt = new ScriptDocumentation();
   tt->SetName(QObject::tr("Timer Types"));
   tt->SetSummary(QObject::tr("Display formats"));
   tt->AddScriptlet(new Scriptlet(QObject::tr("Precise"), "PRECISE",QObject::tr("Value of 0.  About 1ms resolution, never times out early.")));
   tt->AddScriptlet(new Scriptlet(QObject::tr("Coarse"), "COARSE",QObject::tr("Value of 1.  May timeout up to 5% early.")));
   tt->AddScriptlet(new Scriptlet(QObject::tr("Very Coarse"), "VERY_COARSE",QObject::tr("Value of 2.  May timeout up to 500ms early.")));

   d->AddSubDocumentation(tt);
   return d;
}

} // end terbit

