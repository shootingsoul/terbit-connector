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

#include "HexSpinBox53.h"
#include <math.h>
#include <cfloat>
#include <QDoubleSpinBox>
#include <QRegExpValidator>
#include <QRegExp>
#include <QString>
#include <QTextStream>

static uint64_t getMantissa(double val);


HexSpinBox53::HexSpinBox53(QWidget *parent) :
   QDoubleSpinBox(parent)
{
   // make sure to set Validator first!
   m_validator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,14}"),this);
   setDecimals(0);
   m_max53 = pow(FLT_RADIX, DBL_MANT_DIG) - 1;
   m_min53 = 0.0;
   QDoubleSpinBox::setRange(m_min53, m_max53);
}

/////////////////////////////////////////////////////////////////////////////
/// \brief Do not allow decimals
/// \param prec
///
void HexSpinBox53::setDecimals(int prec)
{
   prec;
   QDoubleSpinBox::setDecimals(0);
}

/////////////////////////////////////////////////////////////////////////////
/// \brief Do not allow large nums or decimals
/// \param max
///
void HexSpinBox53::setMaximum(double max)
{
  max = (max > m_max53)?m_max53:max;
  QDoubleSpinBox::setMaximum((double)getMantissa(max));
}

/////////////////////////////////////////////////////////////////////////////
/// \brief Do not allow negative nums or decimals
/// \param min
///
void HexSpinBox53::setMinimum(double min)
{
   min = (min < m_min53)?m_min53:min;
   QDoubleSpinBox::setMinimum((double)getMantissa(min));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Don not allow large nums, negNums or decimals
/// \param val
///
void HexSpinBox53::setSingleStep(double val)
{
   val = (val < m_min53)?m_min53:val;
   val = (val > m_max53)?m_max53:val;
   QDoubleSpinBox::setSingleStep((double)getMantissa(val));
}


void HexSpinBox53::setRange(double min, double max)
{
   min = (min < m_min53)?m_min53:min;
   max = (max > m_max53)?m_max53:max;
   QDoubleSpinBox::setRange((double)getMantissa(min), (double)getMantissa(max));
}

void HexSpinBox53::setMaxLen(uint32_t len)
{
   QString exp;
   QTextStream(&exp) << "[0-9A-Fa-f]{1," << len << "}";
   m_validator->setRegExp(QRegExp(exp));
}



QValidator::State HexSpinBox53::validate(QString &text, int &pos) const
{
   return m_validator->validate(text, pos);
}

double HexSpinBox53::valueFromText(const QString &text) const
{
   return (double)(text.toULongLong(0, 16));
}

QString HexSpinBox53::textFromValue(double val) const
{

   uint64_t intVal = getMantissa(val);
   return QString::number(intVal, 16).toUpper();
}

static uint64_t getMantissa(double val)
{
   // Get the mantissa only
   int exp;
   long double vall = val;
   double tmp = frexp(vall, &exp);
   double tmp2 = tmp * pow(FLT_RADIX, exp);

   return (uint64_t)tmp2;
}
