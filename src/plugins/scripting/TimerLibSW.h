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

#include <QObject>
#include <QtQml/QJSEngine>
#include <stdint.h>
#include <QTimer>
#include "tools/TerbitDefs.h"

#define QT_BEGIN_NAMESPACE

#define QT_END_NAMESPACE
namespace terbit
{

class Workspace;
class ScriptProcessor;
class ScriptDocumentation;

class TimerLibSW : public QObject
{
   Q_OBJECT
public:
   explicit TimerLibSW(Workspace *w, ScriptProcessor *ide, QJSEngine *engine);
   virtual ~TimerLibSW();

   Q_PROPERTY(QJSValue COARSE READ GetTimerTypeCoarse)
   QJSValue GetTimerTypeCoarse() { return Qt::CoarseTimer;}
   Q_PROPERTY(QJSValue VERY_COARSE READ GetTimerTypeVCoarse)
   QJSValue GetTimerTypeVCoarse() { return Qt::VeryCoarseTimer;}
   Q_PROPERTY(QJSValue PRECISE READ GetTimerTypePrecise)
   QJSValue GetTimerTypePrecise() { return Qt::PreciseTimer;}

   Q_INVOKABLE void  Start(int ms){m_timer->start(ms);}
   Q_INVOKABLE void  Start(){m_timer->start();}
   Q_INVOKABLE void  Stop(){m_timer->stop();}
   Q_INVOKABLE int   Interval(){return m_timer->interval();}
   Q_INVOKABLE bool  IsActive(){return m_timer->isActive();}

   Q_INVOKABLE bool	IsSingleShot() const{return m_timer->isSingleShot();}
   Q_INVOKABLE int	RemainingTime() const{return m_timer->remainingTime();}
   Q_INVOKABLE void	SetInterval(int msec){m_timer->setInterval(msec);}
   Q_INVOKABLE void	SetSingleShot(bool singleShot){m_timer->setSingleShot(singleShot);}
   Q_INVOKABLE void	SetTimerType(int atype){m_timer->setTimerType((Qt::TimerType)atype);}
   Q_INVOKABLE int	TimerId() const{return m_timer->timerId();}
   Q_INVOKABLE int	TimerType() const{return m_timer->timerType();}

private:

signals:
   void TimerExpired();

public slots:

private:
   QTimer          *m_timer = NULL;
   Workspace       *m_workspace = NULL;
   ScriptProcessor *m_ide   = NULL;
   QJSEngine       *m_se    = NULL;

};

ScriptDocumentation* BuildScriptDocumentationTimerLibSW();


} // end terbit
