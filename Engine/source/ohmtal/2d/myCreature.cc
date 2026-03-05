#include "myCreature.h"
//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
// MyCreature2D public tom2DSprite
//-----------------------------------------------------------------------------
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mMathFn.h"
#include "core/util/safeDelete.h"

#include "ohmtal/ai/aiMath.h"

#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"
#include "tom2DSprite.h"
#include "tom2DTexture.h"




//#include "tom2D.h"
//#include "tom2DHelper.h"
//#include "tom2DSprite.h"
#include "myCreature.h"


IMPLEMENT_CONOBJECT(MyCreature2D);

MyCreature2D::MyCreature2D()
{
	mFaction		= 0;
	mAge			= 0.f;
	mMaxAge			= 60.f;
	mSex			= 0;
	mStrength		= 1.f;
	mMaxStrength	= 50.f;
	mHealth			= 100.f;
	mMaxHealth		= 100.f;
	mFittness		= 100.f;
	mMaxFittness	= 100.f;
	mSelected		= false;

	mStateId			= 0;
	mTargetId			= 0;

   mChangeMask = 0;
}

MyCreature2D::~MyCreature2D()
{
}

void MyCreature2D::initPersistFields()
{
	Parent::initPersistFields();
	addGroup("MyCreature2D");
	addField("faction", TypeS32, Offset(mFaction, MyCreature2D), "");
	addField("age", TypeF32, Offset(mAge, MyCreature2D), "");
	addField("maxAge", TypeF32, Offset(mMaxAge, MyCreature2D), "");
	addField("sex", TypeS32, Offset(mSex, MyCreature2D), "");
	addField("strength", TypeF32, Offset(mStrength, MyCreature2D), "");
	addField("maxStrength", TypeF32, Offset(mMaxStrength, MyCreature2D), "");
	addField("health", TypeF32, Offset(mHealth, MyCreature2D), "");
	addField("maxHealth", TypeF32, Offset(mMaxHealth, MyCreature2D), "");
	addField("fittness", TypeF32, Offset(mFittness, MyCreature2D), "");
	addField("maxFittness", TypeF32, Offset(mMaxFittness, MyCreature2D), "");
	addField("selected", TypeBool, Offset(mSelected, MyCreature2D), "");
	addField("stateId", TypeS32, Offset(mStateId, MyCreature2D), "");
	addField("target", TypeS32, Offset(mTargetId, MyCreature2D), "");
	endGroup("MyCreature2D");


}
//-----------------------------------------------------------------------------
void MyCreature2D::setChangeMask()
{
   mChangeMask = mStateId + mFaction;
}

//-----------------------------------------------------------------------------
bool MyCreature2D::needUpdateStuff()
{
   return mChangeMask != mStateId + mFaction;
}

//-----------------------------------------------------------------------------

void MyCreature2D::updateStuff()
{

   //INNERCOLOR uses stateID
   /*
  new ScriptObject(Sleep)        { class = "StateSleep";       intId=0; };
  new ScriptObject(Discover)     { class = "StateDiscover";    intId=1; };
  new ScriptObject(LookForFood)  { class = "StateLookForFood"; intId=2; };
  new ScriptObject(GrowUp)       { class = "StateGrowUp";      intId=3; };
  new ScriptObject(sFight)       { class = "StateFight";       intId=4; };
   */
  
   switch (mStateId)
   {
   case 0: //sleep
      mInnerColor = LinearColorF(0.f, 0.f, 0.f, 1.f).toColorI();
      break;
   case 3: //growup
      mInnerColor = LinearColorF(0.f, 1.f, 0.f, 1.f).toColorI();
      break;
   case 4: //sleep
      mInnerColor = LinearColorF(1.f, 0.f, 0.f, 1.f).toColorI();
      break;
   default:
      mInnerColor = LinearColorF(0.7f, 0.7f, 0.7f, 1.f).toColorI();
      break;
   }



   switch (mFaction) {
   case 1:
      mOuterColor = LinearColorF(0.f, 1.f, 0.f, 1.f).toColorI();
      break;
   case 2:
      mOuterColor = LinearColorF(0.25f, 0.5f, 1.f, 1.f).toColorI();
      break;
   case 3:
      mOuterColor = LinearColorF(1.f, 1.f, 0.f, 1.f).toColorI();
      break;
   case 4:
      mOuterColor = LinearColorF(0.75f, 0.75f, 0.75f, 1.f).toColorI();
      break;
   case 5:
      mOuterColor = LinearColorF(1.f, 0.f, 0.f, 1.f).toColorI();
      break;
   case 10:
      mOuterColor = LinearColorF(0.f, 0.f, 0.8f, 1.f).toColorI();
      mInnerColor = LinearColorF(1.f, 0.f, 0.f, mRandF(0.2f, 1.f)).toColorI();
      break;
   default:
      mOuterColor = LinearColorF(1.f, 1.f, 1.f, 1.f).toColorI();
      break;
   }

   setChangeMask();


}
//-----------------------------------------------------------------------------
void MyCreature2D::onUpdate(F32 fDt)
{
   if (needUpdateStuff())
      updateStuff();

   Parent::onUpdate(fDt);
}

//-----------------------------------------------------------------------------
void MyCreature2D::onRender(U32 dt, Point3F lOffset)
{
   if (!mScreen) return;
   if (!mVisible) return;

   if (true)
      renderPrim();
   else
      renderBatch(); //lol same FPS for all this work , prim looks better keep it

}


void MyCreature2D::renderPrim()
{
   if (!mScreen) return;
   if (!mVisible) return;


   if (mSelected)
   {
      LinearColorF lSelectedColor = LinearColorF(0.f, 0.75f, 0.9f, mRandF(0.2f, 1.f));
      mScreen->drawPrimPoint(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x + 4.f, lSelectedColor.toColorI());


   }

   Point2I lPointA;
   Point2I lPointB;

   //HEALTH
   if (mHealth > 0.1f)
   {

      
      S32 lhealthLen = S32(mHealth * this->mCollideRadiusX / 100.f);
      lPointA = Point2I(mX - mCollideRadiusX + 1, mY - mCollideRadiusY - 1);
      lPointB = Point2I(mX - mCollideRadiusX + lhealthLen, mY - mCollideRadiusY - 1);
      mScreen->drawPrimLine(lPointA, lPointB, ColorI(0, 255, 0, 255));
      lPointB = Point2I(mX - mCollideRadiusX + mSize.x - 2, mY - mCollideRadiusY - 1);
      mScreen->drawPrimLine(lPointA, lPointB, ColorI(255, 255, 255, 128));
   }

   //STRENGTH lol look stupid with very high strenght FIXME nailed to 10
   for (S32 i = 0; i < mStrength && i <= 10; i++)
   {
      // mScreen->drawPrimPoint(Point2I(mX - mCollideRadiusX + i, mY - mCollideRadiusY - 2), 1, ColorI(255, 0, 0, 255));
      mScreen->drawPrimTriangle(Point2I(mX - mCollideRadiusX + i, mY - mCollideRadiusY - 2), 1, ColorI(255, 0, 0, 255));
   }

   //	mScreen->drawPrimPoint(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x, mOuterColor);
   ///mScreen->drawPrimPoint(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x / 2.f, mInnerColor);
   mScreen->drawPrimQuad(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x * 2.f, mOuterColor);
   mScreen->drawPrimQuad(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x, mInnerColor);


   MyCreature2D* lTarget = dynamic_cast<MyCreature2D*>(Sim::findObject(mTargetId));
   if (lTarget)
   {
      lPointA = Point2I(mX, mY);
      lPointB = Point2I(lTarget->mX + mRandI(-2, 2), lTarget->mY + mRandI(-2, 2));
      mScreen->drawPrimLine(lPointA, lPointB, mOuterColor);

   }

}

//-----------------------------------------------------------------------------
// draw everything with triangles :D hope that speed up
// batch draw all creatures would be much better !!
//    == tom2dcrl batchTriangleList ?!

void MyCreature2D::renderBatch()
{
	if (!mScreen) return;
	if (!mVisible) return;

   Vector<Point3F> lPoints;
   Vector<ColorI>  lColors;



   // ---- draw selected box -----
   if (mSelected)
   {
      ColorI lSelectedColor = ColorI(0, 192, 230, mRandI(50, 255));
      F32 lSelectedHalfSquare = (this->mSize.x + 4.f / 2);

      GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
         , this->mX - lSelectedHalfSquare
         , this->mX + lSelectedHalfSquare
         , this->mY - lSelectedHalfSquare
         , this->mY + lSelectedHalfSquare
         , lSelectedColor);

   }


   // ---- draw inner and outer of the creature -----
      // OUTER
      // mScreen->drawPrimQuad(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x * 2.f, mOuterColor);
   F32 lHalfHeight = this->mSize.x;
   F32 lX1 = this->mX - lHalfHeight;
   F32 lX2 = this->mX + lHalfHeight;
   F32 lY1 = this->mY - lHalfHeight;
   F32 lY2 = this->mY + lHalfHeight;
   ColorI lColor = mOuterColor;

   GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
      , lX1, lX2, lY1, lY2, lColor);

   // INNER
   // mScreen->drawPrimQuad(Point2I((S32)this->mX, (S32)this->mY), this->mSize.x , mInnerColor);
   lHalfHeight = this->mSize.x / 2.f;
   lX1 = this->mX - lHalfHeight;
   lX2 = this->mX + lHalfHeight;
   lY1 = this->mY - lHalfHeight;
   lY2 = this->mY + lHalfHeight;
   lColor = mInnerColor;

   GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
      , lX1, lX2, lY1, lY2, lColor);



   // ---- draw health bar -----
   if (mHealth > 0.1f)
	{
      ColorI  lHealthColor = ColorI(0, 255, 0, 255);
      ColorI  lBackColor = ColorI(255, 255, 255, 255);
      F32 lBarHalfHeight = 0.5f;

		F32 lhealthLen = mHealth * this->mCollideRadiusX / 100.f;
      F32 x1 = mX - mCollideRadiusX;
      F32 x2 = mX - mCollideRadiusX + lhealthLen;
      F32 y = mY - mCollideRadiusY - 1.f;

      GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
         , x1, x2, y - lBarHalfHeight, y+lBarHalfHeight, lHealthColor);
      x2 = mX - mCollideRadiusX + mSize.x - 2.f;

      GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
         , x1, x2, y - lBarHalfHeight, y + lBarHalfHeight, lBackColor);
	}

   // ---- draw strength bar -----
	//STRENGTH lol look stupid with very high strenght FIXME nailed to 10
	for (S32 i = 0; i < mStrength && i <= 10; i++)
	{
      F32 lBarHalfHeight = 0.5f;
      // mScreen->drawPrimTriangle(Point2I(mX - mCollideRadiusX + i, mY - mCollideRadiusY - 2), 1, ColorI(255, 0, 0, 255));
      GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
         , mX - mCollideRadiusX + i * 1.5f - lBarHalfHeight
         , mX - mCollideRadiusX + i * 1.5f + lBarHalfHeight
         , mY - mCollideRadiusY - 2.f - lBarHalfHeight
         , mY - mCollideRadiusY - 2.f + lBarHalfHeight
         , ColorI(255, 0, 0, 255));
   }

   // ---- draw attack animation  -----

	MyCreature2D* lTarget = dynamic_cast<MyCreature2D*>(Sim::findObject(mTargetId));
	if (lTarget)
	{
      lHalfHeight = 0.4f;
      lX1 = this->mX - lHalfHeight;
      lX2 = lTarget->mX + mRandF(-0.5f,0.5f) + lHalfHeight;
      lY1 = this->mY - lHalfHeight;
      lY2 = lTarget->mY + mRandF(-0.5f, 0.5f) + lHalfHeight;
      lColor = mOuterColor;


      GFX->getDrawUtil()->addQuadTriangleList(lPoints, lColors
         , lX1, lX2, lY1, lY2, lColor);

	}

   //batch render
   getScreen()->addDynamicTrianglePointsAndColors(lPoints, lColors);

}
