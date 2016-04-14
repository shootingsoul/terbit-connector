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

#include <QFile>
#include "Script.h"
#include "Tools.h"
#include <QQmlExpression>
#include <QQmlComponent>

namespace terbit
{


bool ScriptRun(QJSEngine* engine, const QString &script, const QString& source, QString& errorMessage)
{
   bool res = true;

   bool exception = false;
   QJSValue scriptRes;
   try
   {
      scriptRes = engine->evaluate(script, source);
   }
   catch(...)
   {
      //catch whatever we can . . .
      exception = true;
   }

   if (scriptRes.isError())
   {
      res = false;      
      errorMessage = QObject::tr("Script exception occured running script from source %1, Line %2 - %3").arg(source).arg(scriptRes.property("lineNumber").toString()).arg(scriptRes.toString());
   } else if (exception)
   {
      res = false;
      errorMessage = QObject::tr("Unknown script exception occured running script from source %1").arg(source);
   }

   return res;
}

bool ScriptFileRun(QJSEngine* engine, const QString &scriptFile, QString& errorMessage)
{
   bool res = false;
   QFile f(scriptFile);
   if (!f.open(QIODevice::ReadOnly))
   {
      errorMessage = QObject::tr("Script file not found: %1").arg(scriptFile);
   }
   else
   {
      QTextStream stream(&f);
      QString contents = stream.readAll();
      f.close();
      res = ScriptRun(engine, contents, scriptFile, errorMessage);
   }

   return res;
}




QString ScriptEncode(const QString &value)
{
   QString res(value);
   res.replace("\\","\\\\");
   res.replace("\"","\\\"");
   res.prepend('"');
   res.append('"');
   return res;
}

}
