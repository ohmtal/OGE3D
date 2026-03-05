//-----------------------------------------------------------------------------
// Copyright (c) 2009 huehn-software / Ohmtal Game Studio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#ifndef _TOM2DUTILS_H_
#define _TOM2DUTILS_H_


#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _TOM2DTEXTURE_H_
#include "ohmtal/2d/tom2DTexture.h"
#endif


#define HS2D_MAXLAYERS 10000


namespace tom2DUtils
{


   //------------------------------------------------------------------------------
   inline void getCellDrawRects(Point2I* lUpperLeft, Point2I* lExtent, tom2DTexture* img, U32 imgId)
   {
      U32 aCols = img->getCols();
      U32 aRows = img->getRows();

      U32 imgW = img->getTextureHandle()->getWidth();
      U32 imgH = img->getTextureHandle()->getHeight();


      if (aCols == 1 && aRows == 1)
      {
         lUpperLeft->x  = 0;
         lExtent->x = imgW;
         lUpperLeft->y  = 0;
         lExtent->y = imgH;
         return;
      }

      U32 maxCells = aCols * aRows;
      imgId = imgId % maxCells; //failsave imgid

      F32 texW = (F32)imgW / (F32)aCols;
      F32 texH = (F32)imgH / (F32)aRows;

      U32 aColIdx = imgId % aCols;
      U32 aRowIdx = imgId / aCols;

      lUpperLeft->x = aColIdx * texW;
      lExtent->x =  texW;
      lUpperLeft->y = aRowIdx * texH;
      lExtent->y = texH;

   }
   //------------------------------------------------------------------------------
   /*
   inline void getCellDrawRectsWithRect(Point2I* lUpperLeft, Point2I* lLowerRight, tom2DTexture* img, U32 imgId, RectF lSrcRect)
   {
      U32 aCols = img->getCols();
      U32 aRows = img->getRows();


      U32 maxCells = aCols * aRows;
      imgId = imgId % maxCells; //failsave imgid

      F32 texW = 1 / (F32)aCols;
      F32 texH = 1 / (F32)aRows;


      // texW =^= img->getWidth
     // texH =^= img->getHeight
      F32 x1offset = lSrcRect.point.x / img->getWidth();
      F32 x2offset = lSrcRect.extent.x / img->getWidth();
      F32 y1offset = lSrcRect.point.y / img->getHeight();
      F32 y2offset = lSrcRect.extent.y / img->getHeight();


      U32 aColIdx = imgId % aCols;
      U32 aRowIdx = imgId / aCols;

      lUpperLeft->x = aColIdx * texW + x1offset * texW;
      lLowerRight->x = lUpperLeft->x + (texW * x2offset);
      lUpperLeft->y = aRowIdx * texH + y1offset * texH;
      lLowerRight->y = lUpperLeft->y + y2offset * texH;

   }
   */

}; //tom2DHelper


#endif //_TOM2DUTILS_H_
