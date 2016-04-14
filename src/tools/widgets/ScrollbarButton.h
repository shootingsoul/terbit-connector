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

#include <QPushButton>

namespace terbit
{

class ScrollbarButton : public QPushButton
{
public:

   enum ButtonType
   {
      FIRST,
      STEP_PREVIOUS,
      STEP_NEXT,
      LAST
   };

   ScrollbarButton(ButtonType type,Qt::Orientation orientation);

protected:

   virtual void paintEvent(QPaintEvent* event);

private:
   ButtonType m_type;
   Qt::Orientation m_orientation;
};

}
