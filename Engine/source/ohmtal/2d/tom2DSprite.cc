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
// tom2DSprite
//=================================================================================================

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "math/mMathFn.h"
#include "ohmtal/ai/aiMath.h"

#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"
#include "tom2DSprite.h"
#include "tom2DTexture.h"

IMPLEMENT_CONOBJECT(tom2DSprite);

tom2DSprite::tom2DSprite()
{

   mTexture = NULL;

   // example search for mColumnOffsets
   VECTOR_SET_ASSOCIATION(mAnimationFrames);
   mCurFrameIdx = 0;
   mFrameTimer = 0;
   mImgIdx = 0;
   mAnimationDelay = 250;

   mPlayAnimation = false;
   mForceAnimTick = false;

   mVisible = false;
   mRotation = 0;
   mRotateByForwardVector = false;

   mSize = Point2I(32, 32);
   mX = 0.f;
   mY = 0.f;
   mSpeed = 0.f;
   mDirX = 0.f;
   mDirY = 0.f;
   mFlipX = false;
   mFlipY = false;

   mReceiveCollision = false;
   mSendCollision = false;
   mCollideRadiusX = 0.f;
   mCollideRadiusY = 0.f;
   mLastCollideObjectID = 0;

   mDebugRender = false;

   mScreen = 0;

   mForwardVector = Point2F(0.f, 0.f);
   mForwardVectorAngle = 0.f;
   mForwardVectorRad = 0.f;


}

tom2DSprite::~tom2DSprite() {

   if (mScreen) {
      // mScreen->removeSprite(this);
      mScreen->removeRenderObject(this);
      mScreen = NULL;
   }


}

//------------------------------------------------------------------------------

void tom2DSprite::initPersistFields()
{
   Parent::initPersistFields();


   addGroup("tom2DSprite");
   addField("Rotation", TypeF32, Offset(mRotation, tom2DSprite), "float rotation of sprite in degrees");
   addField("RotateByForwardVector", TypeBool, Offset(mRotateByForwardVector, tom2DSprite), "auto rotate by forward vector");
   addField("Size", TypePoint2I, Offset(mSize, tom2DSprite), "Point2I rendering size");
   // addField("Visible", TypeBool,         Offset(mVisible, tom2DSprite),"bool is visible");

   addField("Speed", TypeF32, Offset(mSpeed, tom2DSprite), "float speed - works together with DirX and DirY");
   addField("DirX", TypeF32, Offset(mDirX, tom2DSprite), "float dirX - works together with speed");
   addField("DirY", TypeF32, Offset(mDirY, tom2DSprite), "float dirY - works together with speed");

   addField("FlipX", TypeBool, Offset(mFlipX, tom2DSprite), "bool flipX if true sprite is mirrored on X axis.");
   addField("FlipY", TypeBool, Offset(mFlipY, tom2DSprite), "bool flipY if true sprite is mirrored on Y axis.");
   endGroup("Sprite");

   addGroup("Collision");
   addField("SendCollision", TypeBool, Offset(mSendCollision, tom2DSprite), "bool object checks if it collide with a receiver. Example: Space invader Missile");
   addField("ReceiveCollision", TypeBool, Offset(mReceiveCollision, tom2DSprite), "bool Object will be checked against a collision sender. Example: Space invader Ship");
   addField("CollideRadiusX", TypeF32, Offset(mCollideRadiusX, tom2DSprite), "float X axis radius of collision shape. Check is against a rectancle not circle.");
   addField("CollideRadiusY", TypeF32, Offset(mCollideRadiusY, tom2DSprite), "float Y axis radius of collision shape. Check is against a rectancle not circle.");
   endGroup("Collision");

   addGroup("Animation");
   addField("Frames", TypeS32Vector, Offset(mAnimationFrames, tom2DSprite), "Frames of current animation. Example: 0 1 1 0");
   addField("PlayAnimation", TypeBool, Offset(mPlayAnimation, tom2DSprite), "bool current animation is playing");
   addField("ForceAnimTick", TypeBool, Offset(mForceAnimTick, tom2DSprite), "bool after we changed the frames while animation is running we should set this to true!!");
   addField("AnimationDelay", TypeS32, Offset(mAnimationDelay, tom2DSprite), "U16 Animation Delay default 250, lower = faster animation");
   endGroup("Animation");

   addGroup("Debug");
   addField("DebugRender", TypeBool, Offset(mDebugRender, tom2DSprite), "bool debug render collision");
   endGroup("Debug");

}
//------------------------------------------------------------------------------
bool tom2DSprite::onAdd()
{
   if (!Parent::onAdd())
      return false;


   Con::executef(this, "onAdd", getId());
   return true;
}
//------------------------------------------------------------------------------
void tom2DSprite::onRemove()
{
   Con::executef(this, "onRemove", getId());
   Parent::onRemove();
}
bool tom2DSprite::getPointIn(Point2F pt)
{
   F32 x          = get2DPosition().x;
   F32 halfWidth  = getCollideRadiusX();
   F32 y          = get2DPosition().y;
   F32 halfHeight = getCollideRadiusY();

   return (
      pt.x >= x - halfWidth
      && pt.x < x + halfWidth
      && pt.y >= y - halfHeight
      && pt.y < y + halfHeight
      );
}
//------------------------------------------------------------------------------
// Collision Check 
// This should be optimized using Spatila Partitioning!
//------------------------------------------------------------------------------
void tom2DSprite::checkCollide() {
   if (!mScreen) return;

   // Vector<tom2DSprite *> sprites  = mScreen->getSprites();
   Vector<tom2DRenderObject*> lRenderObjects = mScreen->getRenderObject();
   //Check for collision 
   // Vector<tom2DSprite *>::iterator sprite;
   Vector<tom2DRenderObject*>::iterator lObject;


   S32 foundObjId = 0;
   for (lObject = lRenderObjects.begin(); lObject != lRenderObjects.end(); lObject++)
   {
      tom2DSprite* sprite = dynamic_cast<tom2DSprite*>(*lObject);
      if (!sprite)
         continue;

      F32 myLeft = this->mX - this->mCollideRadiusX;
      F32 myRight = this->mX + this->mCollideRadiusX;
      F32 myTop = this->mY - this->mCollideRadiusY;
      F32 myBottom = this->mY + this->mCollideRadiusY;
      if (sprite != this && sprite->doReceiveCollide()) {
         if (!(
            myLeft > sprite->mX + sprite->mCollideRadiusX ||
            sprite->mX - sprite->mCollideRadiusX > myRight ||

            myTop > sprite->mY + sprite->mCollideRadiusY ||
            sprite->mY - sprite->mCollideRadiusY > myBottom
            )
            )
         {

            foundObjId = sprite->getId();
            break;
         }
      }
   }
   if (foundObjId) {
      if (foundObjId != mLastCollideObjectID) {
         mLastCollideObjectID = foundObjId;
         //Con::executef( this, 2, "onCollision", Con::getIntArg(foundObjId));
         Con::executef(this, "onCollision", foundObjId);
      }
   }
   else {
      mLastCollideObjectID = 0;
   }
}
//------------------------------------------------------------------------------
void tom2DSprite::updateAnimation(F32 fDt)
{


   F32 dt = fDt * 1000.f;

   bool doAnimTick = mForceAnimTick;
   mForceAnimTick = false;
   if (mPlayAnimation) {
      mFrameTimer += dt / mAnimationDelay;
      if (mFrameTimer > 1) {
         mFrameTimer -= 1;
         doAnimTick = true;
      }
   }

   //	mCurFrameIdx

   if (mPlayAnimation && doAnimTick && mAnimationFrames.size() > 0) {
      mCurFrameIdx = (mCurFrameIdx + 1) % mAnimationFrames.size();
      mImgIdx = mAnimationFrames[mCurFrameIdx];
   }

}

//------------------------------------------------------------------------------
void tom2DSprite::onUpdate(F32 fDt)
{

   mForwardVector = Point2F(mDirX, mDirY);
   if (mForwardVector.len() > 0.f)
   {
      /* 
      mRayFeelerLen = mSpeed * fDt;
      if (mRayFeelerLen < 5.f)
         mRayFeelerLen = 5.f;
      */
      mForwardVector.normalize();
      mForwardVectorRad = mAtan2(mDirX, mDirY); 
      mForwardVectorAngle = mRadToDeg(mForwardVectorRad);

      //move
      if (mSpeed > 0.f)
      {

         // mX += (mSpeed * mSin(mForwardVectorRad)) * fDt;
         // mY += (mSpeed * mCos(mForwardVectorRad)) * fDt;
         // should be the same as:
         mX += mSpeed * mDirX * fDt;
         mY += mSpeed * mDirY * fDt;

#ifdef CRAZY_DEBUG

         Con::printf("DEBUG spritemove: obj:%d,Rad:%7.3f,Angle:%7.3f,vecX=%7.3f mx=%7.3f, vecY=%7.3f my=%7.3f"
            , getId()
            , mForwardVectorRad
            , mForwardVectorAngle
            , mForwardVector.x, mX
            , mForwardVector.y, mY);
#endif
      }


   }



   if (mRotateByForwardVector) {
      mRotation = (mForwardVectorAngle + 90.f);
      //mRotation = (U32)(mForwardVectorAngle + 90.f) % 360;
   }


/*
   if (mSpeed > 0.f) {
      mX += mSpeed * fDt * mDirX;
      mY += mSpeed * fDt * mDirY;
   }
*/

   if (doSendCollide()) {
      checkCollide();
   }

   updateAnimation(fDt);

   //Con::executef( this, 2, "onUpdate", Con::getFloatArg(fDt));
   Con::executef(this, "onUpdate", Con::getFloatArg(fDt));

}
//------------------------------------------------------------------------------
void tom2DSprite::onRender(U32 dt, Point3F lOffset)
{

   //XXTH SPRITE DOES NOT USE THE OFFSET ?! ?! ?!

   if (!mScreen) return;

   if (!mVisible) return;

   //Check if on screen
   /* sucks with scrolling ... FIXME!!
      if (
          this->mX+this->mSize.x < 0
          || this->mX > this->mScreen->getBaseExtent().x
          || this->mY + this->mSize.y < 0
          || this->mY > this->mScreen->getBaseExtent().y

        )
         {
            return;
         }
   */

   if (getTexture() && getTexture()->getTextureHandle())
   {
      mScreen->drawStretch(getTexture(), mImgIdx, get3DPosition(), mSize, /*mRotation,*/ mFlipX, mFlipY /*, 0.1f, true*/);
   }
   else {
      //render manually 
      // Con::executef(this, "onRender", getScreen()->getIdString(), Con::getIntArg(dt));
      Con::executef(this,  "onRender", getScreen()->getId(), dt);
   }

}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, setTexture, bool, (tom2DTexture* texObj), , "(tom2DTexture)"
   "set the texture of the sprite")
{
   if (texObj) {
      object->setTexture(texObj);
      return true;
   }
   return false;
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getTexture, S32, (), , "(return tom2DTexture)"
   "set the texture of the sprite")
{
   if (object->getTexture()) {
      return object->getTexture()->getId();
   }
   return 0;
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, setImgIdx, bool, (S32 idx), , "(index of image)"
   "manually setting of image index. Does not work if animation is enabled!")
{
   return object->setImgIdx(idx);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getForwardVector, Point2F, (), , "")
{
   return object->getForwardVector(); 
}
DefineEngineMethod(tom2DSprite, getForwardVectorAngle, F32, (), , "")
{
   return object->getForwardVectorAngle();
}
DefineEngineMethod(tom2DSprite, getForwardVectorRadiant, F32, (), , "")
{
   return object->getForwardVectorRadiant();
}

//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getHalfWidth, F32, (), , "(return F32 )")
{
   return object->getSize().x / 2.f;
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getHalfHeight, F32, (), , "(return F32)")
{
   return object->getSize().y / 2.f;
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, pointIn, bool, (Point2F pt), , "(is the Point2F pt inside the collision radius of the object, return bool)")
{
   return object->getPointIn(pt);
}

//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getPosition, Point2F, (), , "(return Point2I)")
{
   return object->get2DPosition();
}

//------------------------------------------------------------------------------
DefineEngineMethod(tom2DSprite, getDistance, F32,(Point2F pt), , "(distance to the pt. return Point2F)")
{
   return AIMath::PointDistance(object->get2DPosition(),  pt);
}
