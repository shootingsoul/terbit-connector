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
/** This spin box allows the representation of 2^53 (probably)hexadecimal
 *  integers by subclassing a QDoubleSpinBox and using the 53-bit (probably)
 *  mantissa of the double value.
 *  DO NOT access an instance of this class through a pointer to the base
 *  class QDoubleSpinBox.  This can cause really bad behavior in the HexSpinBox53
 *  instance.
 *  Hex adaptation from the book "C++ GUI Programming with Qt 4" by Jasmin
 *  Blanchette and Mark Summerfield.  double to int64_t adaptation from various
 *  places around the web. */

#include <QDoubleSpinBox>
#include <stdint.h>

class QRegExpValidator;

class HexSpinBox53 : public QDoubleSpinBox
{
   Q_OBJECT
public:
   explicit HexSpinBox53(QWidget *parent = 0);

   // The following functions are implemented merely to catch and
   // condition inputs that are valid for QDoubleSpinBox, but are
   // invalid for our integer-based HexSpinBox53
   void setDecimals(int prec);
   void setMaximum(double max);
   void setMinimum(double min);
   void setSingleStep(double val);
   void setRange(double min, double max);
   void setMaxLen(uint32_t len);


protected:
   QValidator::State validate(QString &text, int &pos) const;
   double valueFromText(const QString &text) const;
   QString textFromValue(double val) const;

signals:

public slots:
private:
   QRegExpValidator *m_validator;
   double m_max53;
   double m_min53;

};

