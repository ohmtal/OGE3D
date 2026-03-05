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
// tom2DSprites
//=================================================================================================

#ifndef _TOM2DSPRITE_H_
#define _TOM2DSPRITE_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _TOM2DUTILS_H_
#include "tom2DUtils.h"
#endif

#ifndef _TOM2DRENDEROBJ_H_
#include "ohmtal/2d/tom2DRenderObject.h"
#endif


class tom2DCtrl;




class tom2DSprite : public tom2DRenderObject
{
private:
   typedef tom2DRenderObject Parent;
   //   StringTableEntry mClassName;
   //   StringTableEntry mSuperClassName;

protected:
   tom2DTexture* mTexture;

   Vector<S32> mAnimationFrames;
   U32 mCurFrameIdx;
   U32 mImgIdx;
   F32 mFrameTimer;
   S32 mAnimationDelay; //default 250 
   bool mPlayAnimation;
   bool mForceAnimTick;


   F32 mRotation;
   bool mRotateByForwardVector;

   Point2I mSize;
   F32 mSpeed;
   F32 mDirX;
   F32 mDirY;
   bool mFlipX;
   bool mFlipY;
   bool mReceiveCollision;
   bool mSendCollision;
   F32  mCollideRadiusX;
   F32  mCollideRadiusY;
   S32  mLastCollideObjectID;

   //Readonly 
   Point2F mForwardVector;
   F32     mForwardVectorAngle;
   F32     mForwardVectorRad;


public:
   //creation methods
   DECLARE_CONOBJECT(tom2DSprite);
   tom2DSprite();
   ~tom2DSprite();

   static void initPersistFields();

   void setTexture(tom2DTexture* texture) { mTexture = texture; }
   tom2DTexture* getTexture() { return mTexture; }

   bool setImgIdx(U32 idx) { if (!mPlayAnimation) { mImgIdx = idx; return true; } else return false; }

   void checkCollide();

   void updateAnimation(F32 fDt);

   virtual void onRender(U32 dt, Point3F lOffset);
   virtual void onUpdate(F32 fDt);

   bool onAdd();
   void onRemove();

   Point2I getSize() { return mSize; }

   RectI getRectI() {
      S32 w = getSize().x;
      S32 h = getSize().y;
      return RectI((S32)(get2DPosition().x - w / 2), (S32)(get2DPosition().y - h / 2), w, h);
   }
   //collision stuff


   bool getSendCollision() { return mSendCollision; } //for box2d
   bool doSendCollide() { return (mSendCollision && mCollideRadiusX > 0.f && mCollideRadiusY > 0.f); }
   F32 getCollideRadiusX() { return mCollideRadiusX; }
   F32 getCollideRadiusY() { return mCollideRadiusY; }
   bool doReceiveCollide() {
      return (mReceiveCollision && ((mCollideRadiusX > 0.f && mCollideRadiusY > 0.f) /* || mCollideVertices.size() > 1*/));
   }
   bool getPointIn(Point2F pt);

   //forward vector
   Point2F getForwardVector() { return mForwardVector; }
   F32 getForwardVectorAngle() { return mForwardVectorAngle; }
   F32 getForwardVectorRadiant() { return mForwardVectorRad; }




protected:
   bool mDebugRender;

};
#endif
