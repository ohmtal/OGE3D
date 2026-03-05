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
//-----------------------------------------------------------------------------
#include "console/console.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "sim/actionMap.h"
#include "platform/profiler.h"
#include "core/util/safeDelete.h"
#include "ohmtal/misc/htutils.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "tom2DUtils.h"
#include "tom2DCtrl.h"




//using blend transparency <= i see no difference or ? i keep it like it was

IMPLEMENT_CONOBJECT(tom2DCtrl);

//------------------------------------------------------------------------------
tom2DCtrl::tom2DCtrl()
{
   mLastRenderTime = Platform::getVirtualMilliseconds();
   mMousePos = Point2I(0, 0);


   mBaseExtent = Point2I(800,600); //800*600 is best here!
   mExtentRatio = Point2F(1.f, 1.f);
   mLastExtent.zero();

   mWorldOffset = Point2I(0, 0);

   mImageAlign = 0; //default: center/center, 1: //top left, 2: //top center, 3: //top right, 4: //0=bottom left, 5: //0=bottom center, 6: //0=bottom right
   mTextLayer = 0;

   mCommonDT = 0.016f;

   mUsingWorld = false;
   mCamPos = Point2F(mBaseExtent.x/2, mBaseExtent.y / 2);
   mWorldRect = RectF(0.f, 0.f, mBaseExtent.x / 2, mBaseExtent.y / 2);

   mScaleContent = false; //OGE = true, OGE3D = false :P

   mMousePassThru = false; //2023-12-14


}
//------------------------------------------------------------------------------
void tom2DCtrl::clearRenderObjects()
{
   Vector<tom2DRenderObject*>::iterator pRo = mRenderObjects.begin();
   for (pRo = mRenderObjects.begin(); pRo != mRenderObjects.end(); pRo++)
   {
      (*pRo)->setScreen(NULL);
   }
   mRenderObjects.clear();

  
}
//------------------------------------------------------------------------------
tom2DCtrl::~tom2DCtrl()
{
   clearRenderObjects();

}

//------------------------------------------------------------------------------

bool tom2DCtrl::onAdd()
{
   if (!Parent::onAdd())
      return(false);

   // give all derived access to the fields
   setModStaticFields(true);
   //sucks Con::executef(this, "onAdd", getId());

   return true;
}
//------------------------------------------------------------------------------
void tom2DCtrl::onRemove()
{
   //sucks Con::executef(this, "onRemove", getId());
   Parent::onRemove();

}
//------------------------------------------------------------------------------
// iTickable interface
void tom2DCtrl::interpolateTick(F32 dt)
{
   if (!isAwake()) //2023-04-10
      return;

   //Con::errorf("tom2DCtrl::interpolateTick(F32 dt=%f)", dt);
}

void tom2DCtrl::processTick()
{
   if (!isAwake()) //2023-04-10
      return;

   if (getExtent() != mLastExtent)
   {
      mLastExtent = getExtent();
      Con::executef(this, "onResize", getId());
   }
}

void tom2DCtrl::advanceTime(F32 dt)
{
   if (!isAwake()) //2023-04-10
      return;

   //dt hack!
   if (dt > 0.030f && dt < 0.033f)
   {
      dt = mCommonDT;
   }
   else {
      mCommonDT = dt;
   }


   // Con::executef( this, 2, "onUpdate", Con::getFloatArg(dt));
   Con::executef(this, "onUpdate", Con::getFloatArg(dt));

/* crash a lot:
   Vector<tom2DRenderObject*>::iterator pRo = mRenderObjects.begin();
   for (pRo = mRenderObjects.begin(); pRo != mRenderObjects.end(); pRo++)
   {
      (*pRo)->onUpdate(dt);
   }
*/
   U32 lSize = mRenderObjects.size();
   for (U32 i = 0; i < lSize; i++)
   {
      mRenderObjects[i]->onUpdate(dt);
   }



}


//------------------------------------------------------------------------------

S32 QSORT_CALLBACK tom2DCtrl::compare_RenderObjectDepth(const void* a, const void* b)
{


   const tom2DRenderObject* cp_a = *(const tom2DRenderObject**)a;
   const tom2DRenderObject* cp_b = *(const tom2DRenderObject**)b;

   if (cp_a->getLayer() > cp_b->getLayer())
      return -1;
   else
      return 1;

}
void tom2DCtrl::sortRenderObjects()
{
      dQsort(mRenderObjects.address(), mRenderObjects.size(), sizeof(tom2DRenderObject*), tom2DCtrl::compare_RenderObjectDepth);
}
//------------------------------------------------------------------------------
void tom2DCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   PROFILE_START(tom2DRender);



   if (mScaleContent && updateRect.extent != mBaseExtent)
   {

      /**
       * setup baseextent!
       * view port extent change when we do a set clip
       * with clip: viewport.point.x=0 => extent.x=screenwidth, viewport.point.y=0 => extent.y=screenheight
       * without clip: viewport.extent == updaterect
       * default clip: ==> updaterect
       *
       * LOL setClipRect also calls setViewPort
       */
      GFX->setClipRect(RectI(updateRect.point.x, updateRect.point.y, mBaseExtent.x, mBaseExtent.y));
      GFX->setViewport(RectI(offset.x, offset.y , updateRect.extent.x, updateRect.extent.y));

      //THIS is used for mouse action only
      mExtentRatio.x = mBaseExtent.x / (F32)updateRect.extent.x;
      mExtentRatio.y = mBaseExtent.y / (F32)updateRect.extent.y;

      

   } else {
      mExtentRatio.x = 1.f;
      mExtentRatio.y = 1.f;
   }


   //this dt is a very inconsistent number it usually makes 15 or 16 on my machine but about all 16 ticks a 32
   //this ends up with flicker shapes, we need a better number, so a avarge is build:
   //!!!!!!!!!!!! disable vertical sync FIXIT  !!!!!!!!!!!!!!!!!!!!
   U32 dt = Platform::getVirtualMilliseconds() - mLastRenderTime;

   
   mRenderOffset.x = offset.x;
   mRenderOffset.y = offset.y;
   mRenderOffset.z = 0;


   if (getUsingWorld())
   {
      F32 xDif = mCamPos.x - (U32)(mBaseExtent.x / 2);
      F32 yDif = mCamPos.y - (U32)(mBaseExtent.y / 2);

      if (xDif > mWorldRect.extent.x - mBaseExtent.x)
         xDif = mWorldRect.extent.x - mBaseExtent.x;
      mWorldOffset.x = xDif > 0.f ? xDif : 0.f;

      if (yDif > mWorldRect.extent.y - mBaseExtent.y)
         yDif = mWorldRect.extent.y - mBaseExtent.y;
      mWorldOffset.y = yDif > 0.f ? yDif : 0.f;

      mRenderOffset.x -= mWorldOffset.x;
      mRenderOffset.y -= mWorldOffset.y;
   }

   // Handle with care this is sloooooooooooooooooow but fast prototyping!! 
   Con::executef(this, "onRender", dt);

   //clear the dynamic render batch  should be filled onRender
   mDynamicTrianglePoints.clear();
   mDynamicColors.clear();


   U32 lSize = mRenderObjects.size();
   for (U32 i = 0; i < lSize; i++)
   {
      if (mRenderObjects[i]->getVisible())
         mRenderObjects[i]->onRender(dt, mRenderOffset);
   }


   //2024-01-05 dynamic render batch
   // Vector<Point3F> mDynamicTrianglePoints;
   // Vector<ColorI> mDynamicColors;
   if (mDynamicTrianglePoints.size() > 3)
   {
      GFXVertexBufferHandle<GFXVertexPC> verts(GFX, mDynamicTrianglePoints.size(), GFXBufferTypeVolatile);
      verts.lock();
      for (U32 i = 0; i < mDynamicTrianglePoints.size(); i++)
      {
         verts[i].point.set(mDynamicTrianglePoints[i]);
         verts[i].color.set(mDynamicColors[i]);
      }
      verts.unlock();

      GFX->setStateBlock(GFX->getDrawUtil()->get2DStateBlockRef());
      GFX->setVertexBuffer(verts);
      GFX->setupGenericShaders();
      GFX->drawPrimitive(GFXTriangleList, 0, S32(mDynamicTrianglePoints.size() / 3));

   }

   Parent::onRender(offset, updateRect);
   mLastRenderTime = Platform::getVirtualMilliseconds();


   PROFILE_END();
}

//------------------------------------------------------------------------------
void tom2DCtrl::draw(tom2DTexture* img, F32 x, F32 y, F32 z)
{
   if (!img || !img->getTextureHandle()) {
      Con::errorf("tom2DCtrl::draw invalid image!");
      return;
   }

   //TextureObject* texture = img->getTextureHandle();
   GFXTextureObject* texture = img->getTextureHandle();
   drawStretch(img, 0, Point3F(x, y, z), Point2I(texture->getBitmapWidth(), texture->getBitmapHeight()));
}
//------------------------------------------------------------------------------
F32 lRound(F32 val) {
   if (val - mFloor(val) < 0.5)
      return mFloor(val);
   else
      return mCeil(val);

}
//------------------------------------------------------------------------------
/**
 * Helper for drawStretch imageAlign
 * Only 0 and 1 tested !!! 
 * @since: 2021-03-01
 * default: center/center, 1: top left, 2: top center, 3: top right, 4: bottom left, 5: bottom center, 6: bottom right
*/
void tom2DCtrl::setupImageAlign(Point3F& pos, Point2I dimension)
{
   F32 halfDimensionX = dimension.x / 2;
   F32 halfDimensionY = dimension.y / 2;

   switch (mImageAlign)
   {
   case 1:
      //top left
      //do nothing
      break;
   case 2:
      //top center
      pos.x -= halfDimensionX;
      break;
   case 3:
      //top right
      pos.x -= dimension.x;
      break;
   case 4:
      //bottom left
      pos.y -= dimension.y;
      break;
   case 5:
      //bottom center
      pos.x -= halfDimensionX;
      pos.y -= dimension.y;
      break;
   case 6:
      //bottom right
      pos.x -= dimension.x;
      pos.y -= dimension.y;
      break;
   default:
      // center to absolute
      pos.x -= halfDimensionX;
      pos.y -= halfDimensionY;
      break;
   }




}
//------------------------------------------------------------------------------
void tom2DCtrl::drawStretch(tom2DTexture* img, U32 imgId, Point3F worldPos, Point2I dimension, /*F32 rot,*/
   bool flipX, bool flipY /*, F32 lAlpha, bool doBlend, bool setWhiteColor*/)
{
   if (!img || !img->getTextureHandle()) {
      Con::errorf("tom2DCtrl::drawStretch invalid image!");
      return;
   }


   Point3F pos = mRenderOffset + worldPos;


   setupImageAlign(pos, dimension);

   GFXBitmapFlip lFlip = GFXBitmapFlip_None;
   /*
   enum GFXBitmapFlip
   {
      GFXBitmapFlip_None = 0,
      GFXBitmapFlip_X = 1 << 0,
      GFXBitmapFlip_Y = 1 << 1,
      GFXBitmapFlip_XY = GFXBitmapFlip_X | GFXBitmapFlip_Y
   };
   */
   if (flipX && flipY)
      lFlip = GFXBitmapFlip_XY;
   else if (flipX)
      lFlip = GFXBitmapFlip_X;
   else if (flipY)
      lFlip = GFXBitmapFlip_Y;

   //unused GFXTextureFilterType lFilter = GFXTextureFilterPoint;

   //2024-02-09
   //GFX->getDrawUtil()->clearBitmapModulation();
   ColorI lColor = img->getColor(); //extra var for debug purposes
   GFX->getDrawUtil()->setBitmapModulation(lColor);

   RectI rect(Point2I(pos.x,pos.y), dimension);


   //GFX->getDrawUtil()->drawBitmapStretch(img->getTextureHandle(), rect, lFlip, lFilter);
   
   Point2I lTextUpperLeft, lExtent;
   tom2DUtils::getCellDrawRects(&lTextUpperLeft, &lExtent, img, imgId);

   //void GFXDrawUtil::drawBitmapStretchSR(GFXTextureObject * texture, const RectF & dstRect, const RectF & srcRect, const GFXBitmapFlip in_flip /*= GFXBitmapFlip_None*/, const GFXTextureFilterType filter /*= GFXTextureFilterPoint */, bool in_wrap /*= true*/)
    GFX->getDrawUtil()->drawBitmapStretchSR(img->getTextureHandle(), rect, RectI(lTextUpperLeft, lExtent), lFlip, GFXTextureFilterLinear); //unused , lFilter);
   //lol same GFX->getDrawUtil()->drawBitmapStretch(img->getTextureHandle(), rect, GFXBitmapFlip_None, GFXTextureFilterLinear, false);

 

}
//------------------------------------------------------------------------------
void tom2DCtrl::drawRect(tom2DTexture* img, U32 imgId, Point3F worldPos,
   RectF lSrc, Point2I dimension/*, bool doBlend*/
)
{
   if (!img || !img->getTextureHandle()) {
      Con::errorf("tom2DCtrl::drawRect invalid image!");
      return;
   }

   Point3F pos = mRenderOffset + worldPos;


   setupImageAlign(pos, dimension);


   GFX->getDrawUtil()->clearBitmapModulation();
   GFX->getDrawUtil()->drawBitmap(img->getTextureHandle(), Point2I(pos.x, pos.y));


}
//------------------------------------------------------------------------------

// align (0=left,1=middle,2=right,3=center)
void tom2DCtrl::writeText(const char* text, GuiControlProfile* lProfile, U32 x, U32 y, U32 align, U8 fontcolortype) {



   if (!lProfile->mFont)  //verify the font
   {
      lProfile->mFont = GFont::create(lProfile->mFontType, lProfile->mFontSize, GuiControlProfile::sFontCacheDirectory);
      if (lProfile->mFont == NULL) {
         Con::errorf("Failed to load/create profile font (%s/%d)", lProfile->mFontType, lProfile->mFontSize);
         return;
      }
   }

   Point2I offset = Point2I(mRenderOffset.x + x, mRenderOffset.y + y);

   //nothing to do on left
   switch (align) {
   case 1:	//middle
      offset.x -= U32(lProfile->mFont->getStrWidth(text) / 2);
      break;
   case 2:	//right
      offset.x -= lProfile->mFont->getStrWidth(text);
      break;

   case 3:	//center
      offset.x -= U32(lProfile->mFont->getStrWidth(text) / 2);
      offset.y -= U32(lProfile->mFont->getHeight() / 2);
      break;

   }

   // CHECK x,y on SCREEN! 
   /*FIXME SUCKS ON T3D
   if (!getBounds().pointInRect(offset))
   {
#ifdef TORQUE_DEBUG	
      Con::errorf("Failed to write text. out of bounds (%d,%d)", offset.x, offset.y);
#endif
      return;
   }
*/
   //2022-01-01 did change it on TGE to:
   // CHECK x,y on SCREEN! in window bounds are not absolute screen position but offset is! so i must check bounds against x,y?!
   //2022-01-23 in OGE3D it is the absolute ... ?!?!?
/* 
   Point2I lPos = Point2I(x, y);
   if (!getBounds().pointInRect(lPos)) {
#ifdef TORQUE_DEBUG
      RectI lBounds = getBounds();
      Con::errorf("Failed to write text. out of bounds POS: %d,%d OFFSETPOS: %d,%d BOUNDS: %d,%d %d,%d",
         lPos.x,lPos.y,
         offset.x,offset.y,
         lBounds.point.x,lBounds.point.y,lBounds.extent.x, lBounds.extent.y);
#endif
      return;
   }
*/

   

   switch (fontcolortype)
   {
   case 1:
      GFX->getDrawUtil()->setBitmapModulation(lProfile->mFontColorNA);
      break;
   case 2:
      GFX->getDrawUtil()->setBitmapModulation(lProfile->mFontColorHL);
      break;
   default:
      GFX->getDrawUtil()->setBitmapModulation(lProfile->mFontColor);
      break;
   }

   GFX->getDrawUtil()->drawText(lProfile->mFont, offset, text, mProfile->mFontColors);


}

//------------------------------------------------------------------------------
// align (0=left,1=middle,2=right,3=center)
void tom2DCtrl::writeCustomText(const char* text, U32 x, U32 y, U32 align, const char* fontFace, U32 fontSize, const char* FontColorString)
{

   Resource<GFont> lFont;

   lFont = GFont::create(fontFace, fontSize, GuiControlProfile::sFontCacheDirectory);
   if (lFont == NULL) {
      Con::errorf("Failed to load/create profile font (%s/%d)", fontFace, fontSize);
      return;
   }

   Point2I offset = Point2I(mRenderOffset.x + x, mRenderOffset.y + y);

   //nothing to do on left
   switch (align) {
   case 1:	//middle
      offset.x -= U32(lFont->getStrWidth(text) / 2);
      break;
   case 2:	//right
      offset.x -= lFont->getStrWidth(text);
      break;

   case 3:	//center
      offset.x -= U32(lFont->getStrWidth(text) / 2);
      offset.y -= U32(lFont->getHeight() / 2);
      break;

   }




   // parse color its like 00FF00[00]
   ColorI lColor;
   
   lColor.red = 0;
   lColor.green = 0;
   lColor.blue = 0;
   lColor.alpha = 255;


   F32 r, g, b, a;
   S32 args = dSscanf(FontColorString, "%g %g %g %g", &r, &g, &b, &a);
   if (args >= 3)
   {
      lColor.red = r;
      lColor.green = g;
      lColor.blue = b;
      if (args == 4)
         lColor.alpha = a;
   }

   GFX->getDrawUtil()->setBitmapModulation(lColor);
   GFX->getDrawUtil()->drawText(lFont, offset, text, mProfile->mFontColors);


}
//------------------------------------------------------------------------------
void tom2DCtrl::drawPrimLine(const Point2I& startPt, const Point2I& endPt, const ColorI& color)
{
   GFX->getDrawUtil()->drawLine(startPt, endPt, color);
}
//------------------------------------------------------------------------------
void tom2DCtrl::drawPrimRect(const Point2I& upperL, const Point2I& lowerR, const ColorI& color, bool doFill)
{
   //   void drawRect( const Point2F &upperLeft, const Point2F &lowerRight, const ColorI &color );

   if (doFill)
      GFX->getDrawUtil()->drawRectFill(upperL, lowerR, color);
   else
      GFX->getDrawUtil()->drawRect(upperL, lowerR, color);
}

void tom2DCtrl::drawPrimQuad(const Point2I& center, F32 size, const ColorI& color)
{
   S32 halfSize = (S32)(size / 2);
   Point2I upperL = Point2I(center.x - halfSize, center.y - halfSize);
   Point2I lowerR = Point2I(center.x + halfSize, center.y + halfSize);
   GFX->getDrawUtil()->drawRectFill(upperL, lowerR, color);
}
//------------------------------------------------------------------------------
void tom2DCtrl::drawPrimPoint(const Point2I& center, F32 radius, const ColorI& color, bool smooth)
{
   F32 prec = 0.5f;
   if (smooth)
      prec = 0.1f;
   GFX->getDrawUtil()->draw2DCircleFill(center, radius, color, prec);
}
void tom2DCtrl::drawPrimCircle(const Point2I& center, F32 radius, const ColorI& color, F32 prec)
{
   GFX->getDrawUtil()->draw2DCircleFill(center, radius, color, prec);
}
//------------------------------------------------------------------------------
void tom2DCtrl::drawPrimTriangle(const Point2I& center, F32 size, const ColorI& color)
{
   GFX->getDrawUtil()->draw2DTriangleFill(center, size, color);
}
void tom2DCtrl::drawPrimTriangle(const Point2I& p1, const Point2I& p2, const Point2I& p3, const ColorI& color)
{
   GFX->getDrawUtil()->draw2DTriangleFill(p1,p2,p3, color);
}
//------------------------------------------------------------------------------
void tom2DCtrl::addRenderObject(tom2DRenderObject* obj)
{

   obj->setScreen(this);

   if (find(begin(), end(), obj) == end())
      mRenderObjects.push_back(obj);

   sortRenderObjects();
}


void tom2DCtrl::removeRenderObject(tom2DRenderObject* obj)
{

   Vector<tom2DRenderObject*>::iterator i;

   for (i = mRenderObjects.begin(); i != mRenderObjects.end(); )
   {
      if (*i == obj)
         mRenderObjects.erase(i);
      else
         i++;
   }
}


// DefineEngineMethod(tom2DCtrl, addRenderObject, void, (SimObject* obj), , "addRenderObject(obj1)")
DefineEngineMethod(tom2DCtrl, addRenderObject, void, (tom2DRenderObject* obj), , "addRenderObject(tom2DRenderObject* obj1)")
{
//   if (obj)
//      object->addRenderObject(static_cast<tom2DRenderObject*>(obj));
   if (obj)
      object->addRenderObject(obj);
   else
      Con::printf("Set::add: Object  doesn't exist");
}

// DefineEngineMethod(tom2DCtrl, removeRenderObject, void, (SimObject* obj), , "removeRenderObject(obj1)")
DefineEngineMethod(tom2DCtrl, removeRenderObject, void, (tom2DRenderObject* obj), , "removeRenderObject(tom2DRenderObject* obj1)")
{
   object->lock();
//   if (obj)
//       object->removeRenderObject(static_cast<tom2DRenderObject*>(obj));
   if (obj)
       object->removeRenderObject(obj);
   else
      Con::printf("Set::remove: Object  does not exist in set");
   object->unlock();
}


//NEW 2022-01-23

DefineEngineMethod(tom2DCtrl, clearRenderObjects, void, (), , "clearRenderObjects()")
{
   object->lock();
   object->clearRenderObjects();
   object->unlock();
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------

// adding world for scroller 3013-03-30
//ConsoleMethod( tom2DCtrl, setupWorld, void, 4, 4, "(RectI WorldRect 'x,y,w,h', Point2I CamPos 'x,y'" "setup the world for scrolling")
DefineEngineMethod(tom2DCtrl, setupWorld, void, (RectF area, Point2F campos), , "(RectI WorldRect 'x,y,w,h', Point2I CamPos 'x,y'"
   "setup the world for scrolling")
{

   object->setupWorld(area, campos);
   // Con::printf("World set to %s, CamPos to %s",argv[2],argv[3]);
}

//ConsoleMethod(tom2DCtrl, disableWorld, void, 2, 2, "")
DefineEngineMethod(tom2DCtrl, disableWorld, void, (), , "")
{
   object->setUsingWorld(false);
}



//ConsoleMethod(tom2DCtrl, setCamPos, void, 3, 3, "('x y')")
DefineEngineMethod(tom2DCtrl, setCamPos, void, (Point2F campos), , "('x y')")
{
   object->setCamPos(campos);
}
//ConsoleMethod(tom2DCtrl, moveCam, void, 3, 3, "('x y')")
DefineEngineMethod(tom2DCtrl, moveCam, void, (Point2F camoff), , "('x y')")
{
   //Point2F camoff;
   //dSscanf(argv[2], "%f %f", &camoff.x, &camoff.y);
   object->moveCam(camoff);
}


//ConsoleMethod(tom2DCtrl, getCamPos, const char *, 2, 2, "return pos ")
DefineEngineMethod(tom2DCtrl, getCamPos, const char*, (), , "return pos ")
{
   char* rbuf = Con::getReturnBuffer(256);
   Point2F campos = object->getCamPos();

   dSprintf(rbuf, 256, "%g %g", campos.x, campos.y);
   return rbuf;
}

//ConsoleMethod(tom2DCtrl, getWorldRect, const char *, 2, 2, "return pos ")
DefineEngineMethod(tom2DCtrl, getWorldRect, const char*, (), , "return pos ")
{
   char* rbuf = Con::getReturnBuffer(256);
   RectF rect = object->getWorldRect();

   dSprintf(rbuf, 256, "%g %g %g %g", rect.point.x, rect.point.y, rect.extent.x, rect.extent.y);
   return rbuf;
}


//ConsoleMethod(tom2DCtrl, getWorldOffset, const char *, 2, 2, "return worldoffset")
DefineEngineMethod(tom2DCtrl, getWorldOffset, const char*, (), , "return worldoffset")
{
   char* rbuf = Con::getReturnBuffer(256);
   dSprintf(rbuf, 256, "%d %d", object->getWorldOffset().x, object->getWorldOffset().y);
   return rbuf;

}



//------------------------------------------------------------------------------
//ConsoleMethod( tom2DCtrl, draw, void, 6, 6, "(tom2DTexture,x,y,layer)" "draw a image, Layer 1-99 possible.")
DefineEngineMethod(tom2DCtrl, draw, void, (tom2DTexture* obj, F32 x, F32 y, F32 layer), , "(tom2DTexture,x,y,layer)" "draw a image, Layer 1-99 possible.")
{
   F32 z = layer / HS2D_MAXLAYERS;
   object->draw(obj, x, y, z);
}
/*
ConsoleMethod( tom2DCtrl, drawstretch, void, 9, 14, "(tom2DTexture,imgId,x,y,layer,w,h,[rotation,flipX,flipY, alpha channel default 0.1], optimizetransparent)"
              "draw a image, Layer 1-99 possible. WARNING LAYER DOES NOT WORK ANYMORE!!(iPhone compat)")
{
   tom2DTexture* obj = (tom2DTexture*)Sim::findObject(dAtoi(argv[2]));
    F32 z = dAtof(argv[6]) / HS2D_MAXLAYERS;

    F32  rot   = 0;
   bool flipX = false;
   bool flipY = false;
   F32  lAlpha = 0.1f;
   bool ldoBlend=false;
   if (argc > 9)
       rot = dAtof(argv[9]);
   if (argc > 10)
      flipX = dAtob(argv[10]);
   if (argc > 11)
      flipY = dAtob(argv[11]);
   if (argc > 12)
       lAlpha = dAtof(argv[12]);
   if (argc > 13)
       ldoBlend = dAtob(argv[13]);

    object->drawStretch(obj, dAtoi(argv[3]), Point3F(dAtof(argv[4]),dAtof(argv[5]),z), Point2I(dAtoi(argv[7]),dAtoi(argv[8])),rot , flipX, flipY, lAlpha, ldoBlend);
}
*/


DefineEngineMethod(tom2DCtrl, drawstretch, void, (tom2DTexture* texobj, S32 imgId, F32 x, F32 y, F32 layer, S32 w, S32 h, F32 rot, bool flipX, bool flipY, bool dummy1,bool dummy2,bool dummy3)
   , (0,false,false, false, false, false),
   "(tom2DTexture,imgId,x,y,layer,w,h,[unused rot ,flipX,flipY)"
   "draw a image, Layer 1-99 possible. WARNING LAYER DOES NOT WORK ANYMORE!!(iPhone compat)")
{
   F32 z = layer / HS2D_MAXLAYERS;

   object->drawStretch(texobj, imgId, Point3F(x, y, z), Point2I(w, h), /*rot,*/ flipX, flipY /*, alpha, doBlend*/);
}
DefineEngineMethod(tom2DCtrl, writeText, void, (S32 x, S32 y, const char* string, S32 align,const char* profile, S32 fontcolortype)
   , (0,"", 0)
   ,"(x,y,string, align (0=left,1=middle,2=right,3=center), profile, fontcolortype (0=normal,1=NA,2=HL)"
   "write text on screen.")
{
   GuiControlProfile* lProfile = object->getControlProfile();
   if (profile)
   {
      SimObject* obj = Sim::findObject(profile);
      if (obj)
         lProfile = static_cast<GuiControlProfile*>(obj);
   }
   object->writeText(string, lProfile, x, y, align, fontcolortype);
}
//------------------------------------------------------------------------------

DefineEngineMethod(tom2DCtrl, writeCustomText, void, (S32 x, S32 y, const char* text,S32 align, const char* fontName, S32 fontSize, const char* fontColor), ,
         "(x,y,text, align (0=left,1=middle,2=right,3=center), fontName, fontSize, fontColor ==> like 200 200 200 [255])"
         "write text on screen with font / fontsize and fontcolor.")
{
   //void tom2DCtrl::writeCustomText(const char* text, U32 x, U32 y, U32 align, const char* fontFace, U32 fontSize, const char* FontColorString) 
   object->writeCustomText(text, x, y, align, fontName, fontSize, fontColor);

}


//------------------------------------------------------------------------------
DefineEngineMethod(tom2DCtrl, getMousePosition, const char*, (), , "get the current mouse position")
{
   char* rbuf = Con::getReturnBuffer(256);
   const Point2I lPos = object->getMousePos();
   dSprintf(rbuf, 256, "%d %d", lPos.x, lPos.y);
   return rbuf;

}
//------------------------------------------------------------------------------

void tom2DCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addField("TextLayer", TypeS32, Offset(mTextLayer, tom2DCtrl));
   addField("ImageAlign", TypeS32, Offset(mImageAlign, tom2DCtrl), "default: center/center, 1: //top left, 2: //top center, 3: //top right, 4: //0=bottom left, 5: //0=bottom center, 6: //0=bottom right");
   addField("BaseExtent", TypePoint2I, Offset(mBaseExtent, tom2DCtrl), "BASEExtent should not be bigger than extent, else we get scaling trouble!!");
   addField("ScaleContent", TypeBool, Offset(mScaleContent, tom2DCtrl), "Scale against BaseExtent , default false!");
   addField("MousePassThru", TypeBool, Offset(mMousePassThru, tom2DCtrl), "send mouse events to parent , default false!");
}

//------------------------------------------------------------------------------
void tom2DCtrl::consoleInit()
{
}

//------------------------------------------------------------------------------
/*
static bool isModifierKey(U16 keyCode)
{
   switch (keyCode)
   {
   case KEY_LCONTROL:
   case KEY_RCONTROL:
   case KEY_LALT:
   case KEY_RALT:
   case KEY_LSHIFT:
   case KEY_RSHIFT:
      return(true);
   }

   return(false);
}
*/
//------------------------------------------------------------------------------
bool tom2DCtrl::onInputEvent(const InputEventInfo& event)
{
   /*
      if ( event.objType == SI_BUTTON
        || event.objType == SI_POV
        || ( ( event.objType == SI_KEY ) && !isModifierKey( event.objInst ) ) )
    */ if (true)
    {

       char deviceString[32];
       if (!ActionMap::getDeviceName(event.deviceType, event.deviceInst, deviceString))
          return(false);

       const char* actionStrP = ActionMap::buildActionString(&event);
       char actionString[256];
       dSprintf(actionString, 256, "%s", actionStrP);

       if (event.action == SI_MAKE) {
          //Con::executef(this, 6, "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), "1");
          Con::executef(this, "onInputEvent", deviceString, actionString, mMousePos.x, mMousePos.y, "1");
       }
       else if (event.action == SI_BREAK) {
          //Con::executef(this, 6, "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), "0");
          Con::executef(this, "onInputEvent", deviceString, actionString, mMousePos.x, mMousePos.y, "0");
       }

       else {
          //handle joystick and mouse move! (mouse on non cursor only)
          if ((event.deviceType == JoystickDeviceType || event.deviceType == MouseDeviceType)  /* XXTH this sucks! && event.objInst == 0 */)
          {
             F32 tmpPos = (mFloor(event.fValue * 1000) / 1000);
             if (mFabs(tmpPos) < 0.002)
                tmpPos = 0;
             //Con::executef( this, 7, "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), Con::getFloatArg(tmpPos),Con::getIntArg(event.objType) );
             Con::executef(this, "onInputEvent", deviceString, actionString, mMousePos.x, mMousePos.y, tmpPos, event.objType);
          }
       }

       return(false);
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
void tom2DCtrl::sendMouseEvent(const char* name, const GuiEvent& event)
{
   char buf[3][32];
   dSprintf(buf[0], 32, "%d", event.modifier);
   dSprintf(buf[1], 32, "%d %d", mMousePos.x, mMousePos.y);
   dSprintf(buf[2], 32, "%d", event.mouseClickCount);
   //Con::executef(this, 4, name, buf[0], buf[1], buf[2]);
   Con::executef(this, name, buf[0], buf[1], buf[2]);
}

void tom2DCtrl::setMousePos(const GuiEvent& event)
{
   if (mExtentRatio.x != 1.f && mExtentRatio.y != 1.f)
   {
      mMousePos = Point2I(event.mousePoint.x * mExtentRatio.x, event.mousePoint.y * mExtentRatio.y);
   }
   else {
      mMousePos = Point2I(event.mousePoint.x, event.mousePoint.y);
   }

   //XXTH set mouse cursor relative to window!
   mMousePos -= this->getPosition();

}



void tom2DCtrl::onMouseDragged(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseDragged(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMouseDragged", event);
}

void tom2DCtrl::onMouseEnter(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseEnter(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMouseEnter", event);
}
void tom2DCtrl::onMouseLeave(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseLeave(event);
      }
      return;
   }
   setMousePos(event);
   sendMouseEvent("onMouseLeave", event);
}

void tom2DCtrl::onMouseDown(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseDown(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMouseDown", event);

}

void tom2DCtrl::onMouseUp(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseUp(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMouseUp", event);
}

void tom2DCtrl::onRightMouseDown(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onRightMouseDown(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onRightMouseDown", event);
}

void tom2DCtrl::onRightMouseUp(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onRightMouseUp(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onRightMouseUp", event);

}

void tom2DCtrl::onRightMouseDragged(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onRightMouseDragged(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onRightMouseDragged", event);

}

void tom2DCtrl::onMiddleMouseDown(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMiddleMouseDown(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMiddleMouseDown", event);

}

void tom2DCtrl::onMiddleMouseUp(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMiddleMouseUp(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMiddleMouseUp", event);
}

void tom2DCtrl::onMiddleMouseDragged(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMiddleMouseDragged(event);
      }
      return;
   }

   setMousePos(event);
   sendMouseEvent("onMiddleMouseDragged", event);

}

void tom2DCtrl::onMouseMove(const GuiEvent& event)
{
   if (mMousePassThru)
   {
      GuiControl* parent = getParent();
      if (parent) {
         parent->onMouseMove(event);
      }
      return;
   }
   setMousePos(event);
   sendMouseEvent("onMouseMove", event);
}



//-------------------------------------------------------------------------------------------------
// Cavecat = Cant detect rotation!!! ??? 
tom2DSprite* tom2DCtrl::getSpriteIdAtPos(Point2I pos)
{
   S32 lSize = mRenderObjects.size();
   if (lSize == 0)
      return NULL;

   tom2DSprite* lSprite = NULL;
   RectI lRect(0, 0, 0, 0);
   //reverse search!! 
   for (S32 i = lSize - 1; i >= 0; i--)
   {
      if (mRenderObjects[i]->getVisible())
      {
         lSprite = dynamic_cast<tom2DSprite*>(mRenderObjects[i]);
         if (lSprite) {
            if (lSprite->getRectI().pointInRect(pos))
               return lSprite;
         }
      }
   } //for
   return NULL;
}
//------------------------------------------------------------------------------
// Batch rendering attempt - see also myCreature
//------------------------------------------------------------------------------
bool tom2DCtrl::addDynamicTrianglePointsAndColors(Vector<Point3F> lPoints, Vector<ColorI> lColors)
{
   if (lPoints.size() < 1 || lPoints.size() != lColors.size())
      return false;
   for (U32 i = 0; i < lPoints.size(); i++)
   {
      mDynamicTrianglePoints.push_back(lPoints[i]);
      mDynamicColors.push_back(lColors[i]);

   }
}
//------------------------------------------------------------------------------

DefineEngineMethod(tom2DCtrl, getSpriteIdAtPos, S32, (Point2I pos), , "param Point2I pt")
{
   tom2DSprite* lSprite = object->getSpriteIdAtPos(pos);
   if (lSprite)
      return lSprite->getId();
   return 0;
}


//------------------------------------------------------------------------------
//Tools:
//------------------------------------------------------------------------------
DefineEngineStringlyVariadicFunction(PointInCenterRect, bool, 7, 7, "(Point x,y, Rect: x,y,w,h)"
   "Return bool, it use the center position.")
{
   Point2I myPoint(dAtoi(argv[1]), dAtoi(argv[2]));


   U32  pX = dAtoi(argv[3]);
   U32  pY = dAtoi(argv[4]);
   U32  pW = dAtoi(argv[5]);
   U32  pH = dAtoi(argv[6]);

   RectI myRect(U32(pX - pW / 2), U32(pY - pH / 2), pW, pH);

   return myRect.pointInRect(myPoint);
}


//------------------------------------------------------------------------------
//primitives:
//------------------------------------------------------------------------------
//DefineEngineMethod(tom2DCtrl, addRenderObject, void, (SimObject* obj), , "addRenderObject(obj1)")

DefineEngineMethod(tom2DCtrl, primLine, void, (Point2I startPt, Point2I endPt, LinearColorF color), , "draw a line")
{
   object->drawPrimLine(startPt, endPt, color.toColorI());
}


DefineEngineMethod(tom2DCtrl, primRect, void, (Point2I upperL, Point2I lowerR, LinearColorF color, bool doFill),(false), "draw a rect")
{
   object->drawPrimRect(upperL, lowerR, color.toColorI(), doFill);
}
DefineEngineMethod(tom2DCtrl, primRect2, void, (RectI rect, LinearColorF color, bool doFill), (false), "draw a rect")
{
   object->drawPrimRect(rect.point, rect.point + rect.extent, color.toColorI(), doFill);
}


DefineEngineMethod(tom2DCtrl, PrimQuad, void, (Point2I center, F32 size, LinearColorF color), , "draw a filled quad")
{
   object->drawPrimQuad(center, size, color.toColorI());
}


DefineEngineMethod(tom2DCtrl, primPoint, void, (Point2I center, F32 radius, LinearColorF color, bool smooth ), (true), "draw a point, smooth parameter is obsolete")
{
   object->drawPrimPoint(center, radius, color.toColorI(), smooth);
}

DefineEngineMethod(tom2DCtrl, primCircle, void, (Point2I center, F32 radius, LinearColorF color, F32 prec), (0.1f), "draw a circle, lower prec to get better edges 0.05 for example, BUT: lower is slower!!")
{
   object->drawPrimCircle(center, radius, color.toColorI(), prec);
}



DefineEngineMethod(tom2DCtrl, primTriangle, void, (Point2I center, F32 size, LinearColorF color), , "draw a point")
{
   object->drawPrimTriangle(center, size, color.toColorI());
}

DefineEngineMethod(tom2DCtrl, primTriangle2, void, (Point2I p1, Point2I p2, Point2I p3, LinearColorF color), , "draw a point")
{
   object->drawPrimTriangle(p1,p2,p3, color.toColorI());
}


//------------------------------------------------------------------------------
DefineEngineMethod(tom2DCtrl, getBaseWidth, S32, (), , "get the base width")
{

   return object->getBaseExtent().x;
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DCtrl, getBaseHeight, S32, (), , "get the base width")
{

   return object->getBaseExtent().y;
}
//------------------------------------------------------------------------------
