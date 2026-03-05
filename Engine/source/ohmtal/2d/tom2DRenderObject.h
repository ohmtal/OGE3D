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
//=================================================================================================
// tom2DRenderObject
//=================================================================================================
#ifndef _TOM2DRENDEROBJ_H_
#define _TOM2DRENDEROBJ_H_



#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _TOM2DUTILS_H_
#include "tom2DUtils.h"
#endif



class tom2DCtrl;

class tom2DRenderObject : public SimObject
{
private:
   typedef SimObject Parent;
protected:
   tom2DCtrl* mScreen;
   U32 mLayer;
   F32 mX;
   F32 mY;
   bool mVisible;

public:
   tom2DRenderObject() {
      mScreen = NULL;
      mLayer = 10;
      mX = 0.f; mY = 0.f;
      mVisible = true;

   }

   static void initPersistFields();

   virtual void onRender(U32 dt, Point3F lOffset) {};
   virtual void onUpdate(F32 fDt) {};

   void onRemove();

   virtual void setScreen(tom2DCtrl* screen) { mScreen = screen; }
   virtual tom2DCtrl* getScreen() { return mScreen; }

   virtual void setLayer(U32 lLayer);

   virtual U32 setLayerZ() { return (U32)((F32)mLayer / HS2D_MAXLAYERS); }

   virtual U32 getLayer() const {
      return mLayer;
   }

   virtual F32 getLayerValue() {
      return (F32)mLayer / HS2D_MAXLAYERS;
   }
   virtual Point2F get2DPosition() {
      return Point2F(mX, mY);
   }
   virtual Point3F get3DPosition() {
      return Point3F(mX, mY, getLayerValue());
   }

   virtual bool getVisible() { return mVisible; }


public:
   // Layer Property Accessors
   static bool staticSetLayer(void* obj, const char* index, const char* data) {
      //static_cast<tom2DRenderObject*>(obj)->setLayer((U8)data);
      S32 lLayer = dAtoi(data);
      static_cast<tom2DRenderObject*>(obj)->setLayer((U32)lLayer);

      return true;
   }
   static const char* staticGetLayer(void* obj, const char* index, const char* data) {
      char* returnBuffer = Con::getReturnBuffer(8);
      U32 lLayer = static_cast<tom2DRenderObject*>(obj)->getLayer();
      dSprintf(returnBuffer, 8, "%d", lLayer);

      return returnBuffer;
   }


};



#endif
