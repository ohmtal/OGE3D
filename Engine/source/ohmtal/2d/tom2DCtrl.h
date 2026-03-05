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
#ifndef _TOM2DCTRL_H_
#define _TOM2DCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _GUIMOUSEEVENTCTRL_H_
#include "gui/utility/guiMouseEventCtrl.h"
#endif

#ifndef _GFXTEXTUREMANAGER_H_
#include "gfx/gfxTextureManager.h"
#endif


#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif


#ifndef _TOM2DRENDEROBJ_H_
#include "ohmtal/2d/tom2DRenderObject.h"
#endif

#ifndef _TOM2DTEXTURE_H_
#include "ohmtal/2d/tom2DTexture.h"
#endif

#ifndef _TOM2DSPRITE_H_
#include "tom2DSprite.h"
#endif



//=================================================================================================
// tom2DCtrl
//=================================================================================================
class tom2DCtrl : public GuiMouseEventCtrl, public ITickable
{

   typedef GuiMouseEventCtrl Parent;



protected:
   bool onInputEvent(const InputEventInfo& event);


   void setMousePos(const GuiEvent& event);


   // object selection additions
   virtual void onMouseMove(const GuiEvent& evt);
   virtual void onMouseDragged(const GuiEvent& event);
   virtual void onMouseEnter(const GuiEvent& event);
   virtual void onMouseLeave(const GuiEvent& event);
   virtual void onMouseDown(const GuiEvent& event);
   virtual void onMouseUp(const GuiEvent& event);
   virtual void onRightMouseDown(const GuiEvent& event);
   virtual void onRightMouseUp(const GuiEvent& event);
   virtual void onRightMouseDragged(const GuiEvent& event);
   void onMiddleMouseDown(const GuiEvent& event);
   void onMiddleMouseUp(const GuiEvent& event);
   void onMiddleMouseDragged(const GuiEvent& event);



   void sendMouseEvent(const char* name, const GuiEvent&);


protected:
   //render timing
   U32 mLastRenderTime;

   F32 mCommonDT;

   Point2I mMousePos;

   bool    mScaleContent; //2023-04-07
   Point2I mBaseExtent;
   Point2I mLastExtent;  //2023-04-09 detect resize!
   Point2F mExtentRatio;

   bool mMousePassThru;

   Vector<tom2DRenderObject*> mRenderObjects;


   S32 mTextLayer;
   S32 mImageAlign;

   // adding world for scroller 3013-03-30
   bool mUsingWorld;
   RectF mWorldRect;
   Point2F mCamPos;


   //iTickable interface
   virtual void interpolateTick(F32 dt);
   virtual void processTick();
   virtual void advanceTime(F32 dt);

private:
   Point2I mWorldOffset;
   Point3F mRenderOffset;
   static S32 QSORT_CALLBACK compare_RenderObjectDepth(const void* a, const void* b);

   void setupImageAlign(Point3F& pos, Point2I dimension);

public:

   tom2DCtrl();
   ~tom2DCtrl();

   // SimObject
   bool onAdd();
   void onRemove();


   static void initPersistFields();
   static void consoleInit();

   Point2I getWorldOffset() { return mWorldOffset; }
   Point3F getRenderOffset() { return mRenderOffset; }


   // guiControl
   virtual void onRender(Point2I offset, const RectI& updateRect);

   void draw(tom2DTexture* img, F32 x, F32 y, F32 z);
   void drawStretch(tom2DTexture* img, U32 imgId, Point3F worldPos, Point2I dimension, /*F32 rot = 0,*/ bool flipX = false, bool flipY = false); //, F32 lAlpha = 0.1f, bool doBlend = false, bool setWhiteColor = true);
   void drawRect(tom2DTexture* img, U32 imgId, Point3F worldPos, RectF lSrc, Point2I dimension); //, bool doBlend);


   //fontcolortype: fontcolortype (0=normal,1=NA,2=HL)
   void writeText(const char* text, GuiControlProfile* lProfile, U32 x, U32 y, U32 align = 0, U8 fontcolortype = 0);
   void writeCustomText(const char* text, U32 x, U32 y, U32 align, const char* fontFace, U32 fontSize, const char* FontColorString);

   // bool PointInImage(RectI rect, Point2I pointpos);
   // bool MousePointInImage(RectI rect);

   Point2I getMousePos() { return mMousePos; }


   //primitive Rendering
   void drawPrimLine(const Point2I& startPt, const Point2I& endPt, const ColorI& color);
   void drawPrimRect(const Point2I& upperL, const Point2I& lowerR, const ColorI& color, bool doFill = false);
   void drawPrimQuad(const Point2I& center, F32 size, const ColorI& color);
   void drawPrimPoint(const Point2I& center, F32 radius, const ColorI& color, bool smooth = false);
   void drawPrimCircle(const Point2I& center, F32 radius, const ColorI& color, F32 prec);
   void drawPrimTriangle(const Point2I& center, F32 size, const ColorI& color);
   void drawPrimTriangle(const Point2I& p1, const Point2I& p2, const Point2I& p3, const ColorI& color);

   // adding world for scroller 3013-03-30
   void setupWorld(RectF lWorldRect, Point2F lCamPos)
   {
      mWorldRect = lWorldRect;
      mCamPos = lCamPos;
      mUsingWorld = true;
   }

   bool getUsingWorld() { return mUsingWorld; }
   void setUsingWorld(bool value) { mUsingWorld = value; }
   RectF getWorldRect() { return mWorldRect; }
   Point2F getCamPos() { return mCamPos; }
   void setCamPos(Point2F lCamPos) {
      mCamPos = lCamPos;
      if (mCamPos.x < mWorldRect.point.x)
         mCamPos.x = mWorldRect.point.x;
      else if (mCamPos.x > mWorldRect.point.x + mWorldRect.extent.x)
         mCamPos.x = mWorldRect.point.x + mWorldRect.extent.x;

      if (mCamPos.y < mWorldRect.point.y)
         mCamPos.y = mWorldRect.point.y;
      else if (mCamPos.y > mWorldRect.point.y + mWorldRect.extent.y)
         mCamPos.y = mWorldRect.point.y + mWorldRect.extent.y;

   }
   void moveCam(Point2F lCamOffset) { setCamPos(mCamPos + lCamOffset); }

   //----

   void sortRenderObjects();
   void addRenderObject(tom2DRenderObject* obj);
   void removeRenderObject(tom2DRenderObject* obj);
   Vector<tom2DRenderObject*> getRenderObject() { return mRenderObjects; }
   void clearRenderObjects();

   Point2I getBaseExtent() { return  mBaseExtent; }

   tom2DSprite* getSpriteIdAtPos(Point2I pos);



   //batch render attempt
protected:
   Vector<Point3F> mDynamicTrianglePoints;
   Vector<ColorI> mDynamicColors;
public:
   bool addDynamicTrianglePointsAndColors(Vector<Point3F> lPoints, Vector<ColorI> lColors);

public:
   DECLARE_CONOBJECT(tom2DCtrl);
   DECLARE_CATEGORY("Gui 2D"); //THIS SET THE CATEGORY 2D IN THE EDITOR !!
};


#endif // _TOM2D_H_
