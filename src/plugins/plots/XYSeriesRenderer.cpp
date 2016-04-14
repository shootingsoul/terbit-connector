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
#include "XYSeriesRenderer.h"
#include "connector-core/LogDL.h"

namespace terbit
{

XYSeriesRenderer::XYSeriesRenderer(XYSeries* series) : m_series(series)
{
}

template<typename DataTypeX, typename DataTypeY>
void XYSeriesRenderLayer2(QPainter* painter, const XYSeriesRenderArea& area, DataTypeX* X, size_t strideX, DataTypeY* Y, size_t strideY, size_t start, size_t end)
{

   int x1, y1, x2, y2;
   //TODO improve on this . . . . TerbitValues or something . . . .
   //convert everything to double for the time being in case we are mix/matching data types between series
   double valueCurrentX, startX;
   double valueCurrentY, startY;
   double rangeX, rangeY; //kept range as double because it's used for scaling and value difference (e.g. int8 64 - -100 = 164 overflow for int8)


   startX = area.startX;
   rangeX = area.rangeX;
   startY = area.startY;
   rangeY = area.rangeY;

   if (sizeof(DataTypeX) == strideX && sizeof(DataTypeY) == strideY)
   {
      size_t i = start;
      X = (DataTypeX*)((char*)X + i*strideX);
      Y = (DataTypeY*)((char*)Y + i*strideY);

      valueCurrentX = (double)*X;
      valueCurrentY = (double)*Y;

      x1 = ScaleDataToLogical(valueCurrentX, area.rectX, area.rectW, startX, rangeX);
      y1 = ScaleDataToLogicalReverse(valueCurrentY, area.rectY, area.rectH, startY, rangeY);

      ++X;
      ++Y;
      ++i;

      while(i <= end)
      {
         valueCurrentX = (double)*X;
         valueCurrentY = (double)*Y;

         x2 = ScaleDataToLogical(valueCurrentX, area.rectX, area.rectW, startX, rangeX);
         y2 = ScaleDataToLogicalReverse(valueCurrentY, area.rectY, area.rectH, startY, rangeY);

         painter->drawLine(x1, y1, x2, y2);

         ++X;
         ++Y;
         ++i;

         x1 = x2;
         y1 = y2;
      }
   }
   else
   {
      size_t i = start;
      X = (DataTypeX*)((char*)X + i*strideX);
      Y = (DataTypeY*)((char*)Y + i*strideY);

      valueCurrentX = (double)*X;
      valueCurrentY = (double)*Y;

      x1 = ScaleDataToLogical(valueCurrentX, area.rectX, area.rectW, startX, rangeX);
      y1 = ScaleDataToLogicalReverse(valueCurrentY, area.rectY, area.rectH, startY, rangeY);

      X = (DataTypeX*)((char*)X + strideX);
      Y = (DataTypeY*)((char*)Y + strideY);
      ++i;

      while(i <= end)
      {
         valueCurrentX = (double)*X;
         valueCurrentY = (double)*Y;

         x2 = ScaleDataToLogical(valueCurrentX, area.rectX, area.rectW, startX, rangeX);
         y2 = ScaleDataToLogicalReverse(valueCurrentY, area.rectY, area.rectH, startY, rangeY);

         painter->drawLine(x1, y1, x2, y2);

         X = (DataTypeX*)((char*)X + strideX);
         Y = (DataTypeY*)((char*)Y + strideY);
         ++i;

         x1 = x2;
         y1 = y2;
      }
   }
}

template<typename DataTypeX>
void XYSeriesRenderLayer1(QPainter* painter, const XYSeriesRenderArea& area, DataTypeX* X, size_t strideX, DataSet* Y, size_t start, size_t end)
{
   switch (Y->GetDataType())
   {
   case TERBIT_DOUBLE:
      XYSeriesRenderLayer2<DataTypeX,double>(painter,area,X,strideX,(double*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_FLOAT:
      XYSeriesRenderLayer2<DataTypeX,float>(painter,area,X,strideX,(float*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_INT8:
      XYSeriesRenderLayer2<DataTypeX,int8_t>(painter,area,X,strideX,(int8_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_INT16:
      XYSeriesRenderLayer2<DataTypeX,int16_t>(painter,area,X,strideX,(int16_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_INT32:
      XYSeriesRenderLayer2<DataTypeX,int32_t>(painter,area,X,strideX,(int32_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_INT64:
      XYSeriesRenderLayer2<DataTypeX,int64_t>(painter,area,X,strideX,(int64_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_UINT8:
      XYSeriesRenderLayer2<DataTypeX,uint8_t>(painter,area,X,strideX,(uint8_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_UINT16:
      XYSeriesRenderLayer2<DataTypeX,uint16_t>(painter,area,X,strideX,(uint16_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_UINT32:
      XYSeriesRenderLayer2<DataTypeX,uint32_t>(painter,area,X,strideX,(uint32_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   case TERBIT_UINT64:
      XYSeriesRenderLayer2<DataTypeX,uint64_t>(painter,area,X,strideX,(uint64_t*)Y->GetBufferAddress(),Y->GetStrideBytes(),start,end);
      break;
   }
}

void XYSeriesRenderer::Render(QPainter* painter, XYSeriesRenderArea& area, DataSet* X, DataSet* Y, size_t start, size_t end)
{
   if (start < end)
   {
      painter->setPen(m_series->GetColor());

      switch (X->GetDataType())
      {
      case TERBIT_DOUBLE:
         XYSeriesRenderLayer1<double>(painter,area,(double*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_FLOAT:
         XYSeriesRenderLayer1<float>(painter,area,(float*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_INT8:
         XYSeriesRenderLayer1<int8_t>(painter,area,(int8_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_INT16:
         XYSeriesRenderLayer1<int16_t>(painter,area,(int16_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_INT32:
         XYSeriesRenderLayer1<int32_t>(painter,area,(int32_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_INT64:
         XYSeriesRenderLayer1<int64_t>(painter,area,(int64_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_UINT8:
         XYSeriesRenderLayer1<uint8_t>(painter,area,(uint8_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_UINT16:
         XYSeriesRenderLayer1<uint16_t>(painter,area,(uint16_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_UINT32:
         XYSeriesRenderLayer1<uint32_t>(painter,area,(uint32_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      case TERBIT_UINT64:
         XYSeriesRenderLayer1<uint64_t>(painter,area,(uint64_t*)X->GetBufferAddress(),X->GetStrideBytes(), Y, start,end);
         break;
      }
   }
}

}
