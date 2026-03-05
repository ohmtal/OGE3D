//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Copyright (c) 20?? part of the opensourced MMOKit 
// Copyright (c) 2008? Ohmtal Game Studio
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

/*

new TSDynamic(dingsObj) {          SHAPEname = "art/shapes/actors/mrbox/player.dts"; };
MissionGroup.add(dingsObj);

RenderShapeExample is a good place to look.
Shadow and reflection does not work, guess it's a problem with the big Worldbox but dunno
at the moment disabled;


*/

//-----------------------------------------------------------------------------

#include "fxTSDynamic.h"

#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightQuery.h"


#include "ts/tsShapeInstance.h"
#include "console/consoleTypes.h"
#include "sim/netConnection.h"
#include "math/mathUtils.h"
#include "ohmtal/ai/aiMath.h"

#include "ohmtal/app/globals.h"

/* 
#include "core/bitStream.h"
#include "dgl/dgl.h"
#include "sceneGraph/sceneState.h"
#include "sceneGraph/sceneGraph.h"
#include "game/shapeBase.h"
#include "game/shadow.h"
#include "sceneGraph/detailManager.h"
#include "game/gameConnection.h"
*/

IMPLEMENT_CO_NETOBJECT_V1(TSDynamic);


static F32	gDirs[162][3] = {
   {-0.525731f, 0.000000f, 0.850651f}, {-0.442863f, 0.238856f, 0.864188f},
   {-0.295242f, 0.000000f, 0.955423f}, {-0.309017f, 0.500000f, 0.809017f},
   {-0.162460f, 0.262866f, 0.951056f}, {0.000000f, 0.000000f, 1.000000f},
   {0.000000f, 0.850651f, 0.525731f}, {-0.147621f, 0.716567f, 0.681718f},
   {0.147621f, 0.716567f, 0.681718f}, {0.000000f, 0.525731f, 0.850651f},
   {0.309017f, 0.500000f, 0.809017f}, {0.525731f, 0.000000f, 0.850651f},
   {0.295242f, 0.000000f, 0.955423f}, {0.442863f, 0.238856f, 0.864188f},
   {0.162460f, 0.262866f, 0.951056f}, {-0.681718f, 0.147621f, 0.716567f},
   {-0.809017f, 0.309017f, 0.500000f}, {-0.587785f, 0.425325f, 0.688191f},
   {-0.850651f, 0.525731f, 0.000000f}, {-0.864188f, 0.442863f, 0.238856f},
   {-0.716567f, 0.681718f, 0.147621f}, {-0.688191f, 0.587785f, 0.425325f},
   {-0.500000f, 0.809017f, 0.309017f}, {-0.238856f, 0.864188f, 0.442863f},
   {-0.425325f, 0.688191f, 0.587785f}, {-0.716567f, 0.681718f, -0.147621f},
   {-0.500000f, 0.809017f, -0.309017f}, {-0.525731f, 0.850651f, 0.000000f},
   {0.000000f, 0.850651f, -0.525731f}, {-0.238856f, 0.864188f, -0.442863f},
   {0.000000f, 0.955423f, -0.295242f}, {-0.262866f, 0.951056f, -0.162460f},
   {0.000000f, 1.000000f, 0.000000f}, {0.000000f, 0.955423f, 0.295242f},
   {-0.262866f, 0.951056f, 0.162460f}, {0.238856f, 0.864188f, 0.442863f},
   {0.262866f, 0.951056f, 0.162460f}, {0.500000f, 0.809017f, 0.309017f},
   {0.238856f, 0.864188f, -0.442863f}, {0.262866f, 0.951056f, -0.162460f},
   {0.500000f, 0.809017f, -0.309017f}, {0.850651f, 0.525731f, 0.000000f},
   {0.716567f, 0.681718f, 0.147621f}, {0.716567f, 0.681718f, -0.147621f},
   {0.525731f, 0.850651f, 0.000000f}, {0.425325f, 0.688191f, 0.587785f},
   {0.864188f, 0.442863f, 0.238856f}, {0.688191f, 0.587785f, 0.425325f},
   {0.809017f, 0.309017f, 0.500000f}, {0.681718f, 0.147621f, 0.716567f},
   {0.587785f, 0.425325f, 0.688191f}, {0.955423f, 0.295242f, 0.000000f},
   {1.000000f, 0.000000f, 0.000000f}, {0.951056f, 0.162460f, 0.262866f},
   {0.850651f, -0.525731f, 0.000000f}, {0.955423f, -0.295242f, 0.000000f},
   {0.864188f, -0.442863f, 0.238856f}, {0.951056f, -0.162460f, 0.262866f},
   {0.809017f, -0.309017f, 0.500000f}, {0.681718f, -0.147621f, 0.716567f},
   {0.850651f, 0.000000f, 0.525731f}, {0.864188f, 0.442863f, -0.238856f},
   {0.809017f, 0.309017f, -0.500000f}, {0.951056f, 0.162460f, -0.262866f},
   {0.525731f, 0.000000f, -0.850651f}, {0.681718f, 0.147621f, -0.716567f},
   {0.681718f, -0.147621f, -0.716567f}, {0.850651f, 0.000000f, -0.525731f},
   {0.809017f, -0.309017f, -0.500000f}, {0.864188f, -0.442863f, -0.238856f},
   {0.951056f, -0.162460f, -0.262866f}, {0.147621f, 0.716567f, -0.681718f},
   {0.309017f, 0.500000f, -0.809017f}, {0.425325f, 0.688191f, -0.587785f},
   {0.442863f, 0.238856f, -0.864188f}, {0.587785f, 0.425325f, -0.688191f},
   {0.688191f, 0.587785f, -0.425325f}, {-0.147621f, 0.716567f, -0.681718f},
   {-0.309017f, 0.500000f, -0.809017f}, {0.000000f, 0.525731f, -0.850651f},
   {-0.525731f, 0.000000f, -0.850651f}, {-0.442863f, 0.238856f, -0.864188f},
   {-0.295242f, 0.000000f, -0.955423f}, {-0.162460f, 0.262866f, -0.951056f},
   {0.000000f, 0.000000f, -1.000000f}, {0.295242f, 0.000000f, -0.955423f},
   {0.162460f, 0.262866f, -0.951056f}, {-0.442863f, -0.238856f, -0.864188f},
   {-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f},
   {0.000000f, -0.850651f, -0.525731f}, {-0.147621f, -0.716567f, -0.681718f},
   {0.147621f, -0.716567f, -0.681718f}, {0.000000f, -0.525731f, -0.850651f},
   {0.309017f, -0.500000f, -0.809017f}, {0.442863f, -0.238856f, -0.864188f},
   {0.162460f, -0.262866f, -0.951056f}, {0.238856f, -0.864188f, -0.442863f},
   {0.500000f, -0.809017f, -0.309017f}, {0.425325f, -0.688191f, -0.587785f},
   {0.716567f, -0.681718f, -0.147621f}, {0.688191f, -0.587785f, -0.425325f},
   {0.587785f, -0.425325f, -0.688191f}, {0.000000f, -0.955423f, -0.295242f},
   {0.000000f, -1.000000f, 0.000000f}, {0.262866f, -0.951056f, -0.162460f},
   {0.000000f, -0.850651f, 0.525731f}, {0.000000f, -0.955423f, 0.295242f},
   {0.238856f, -0.864188f, 0.442863f}, {0.262866f, -0.951056f, 0.162460f},
   {0.500000f, -0.809017f, 0.309017f}, {0.716567f, -0.681718f, 0.147621f},
   {0.525731f, -0.850651f, 0.000000f}, {-0.238856f, -0.864188f, -0.442863f},
   {-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f},
   {-0.850651f, -0.525731f, 0.000000f}, {-0.716567f, -0.681718f, -0.147621f},
   {-0.716567f, -0.681718f, 0.147621f}, {-0.525731f, -0.850651f, 0.000000f},
   {-0.500000f, -0.809017f, 0.309017f}, {-0.238856f, -0.864188f, 0.442863f},
   {-0.262866f, -0.951056f, 0.162460f}, {-0.864188f, -0.442863f, 0.238856f},
   {-0.809017f, -0.309017f, 0.500000f}, {-0.688191f, -0.587785f, 0.425325f},
   {-0.681718f, -0.147621f, 0.716567f}, {-0.442863f, -0.238856f, 0.864188f},
   {-0.587785f, -0.425325f, 0.688191f}, {-0.309017f, -0.500000f, 0.809017f},
   {-0.147621f, -0.716567f, 0.681718f}, {-0.425325f, -0.688191f, 0.587785f},
   {-0.162460f, -0.262866f, 0.951056f}, {0.442863f, -0.238856f, 0.864188f},
   {0.162460f, -0.262866f, 0.951056f}, {0.309017f, -0.500000f, 0.809017f},
   {0.147621f, -0.716567f, 0.681718f}, {0.000000f, -0.525731f, 0.850651f},
   {0.425325f, -0.688191f, 0.587785f}, {0.587785f, -0.425325f, 0.688191f},
   {0.688191f, -0.587785f, 0.425325f}, {-0.955423f, 0.295242f, 0.000000f},
   {-0.951056f, 0.162460f, 0.262866f}, {-1.000000f, 0.000000f, 0.000000f},
   {-0.850651f, 0.000000f, 0.525731f}, {-0.955423f, -0.295242f, 0.000000f},
   {-0.951056f, -0.162460f, 0.262866f}, {-0.864188f, 0.442863f, -0.238856f},
   {-0.951056f, 0.162460f, -0.262866f}, {-0.809017f, 0.309017f, -0.500000f},
   {-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f},
   {-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f},
   {-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f},
   {-0.688191f, 0.587785f, -0.425325f}, {-0.587785f, 0.425325f, -0.688191f},
   {-0.425325f, 0.688191f, -0.587785f}, {-0.425325f, -0.688191f, -0.587785f},
   {-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f},
};



//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
TSDynamic::TSDynamic()
{
   mNetFlags.set(Ghostable| ScopeAlways);
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
#ifdef OGE_ENVCACHE
/*
	if (!OhmtalGlobals::getDevelMode())
		mNetFlags.clear(Ghostable | ScopeAlways);
*/
#endif

   mShapeName = "";
   mAnimation = "";
   mShapeInstance    = NULL;
   mThread = NULL;
   mLastTime = 0xFFFFFFFF;

   mCount = 1;
   mRadiusX = 10.f;
   mRadiusY = 10.f;
   mRadiusZ = 2.f;

   mSpeed = 0.2f;
   mMyScale = 1.f;

   mFadeOutDistance = 150.f;
   mLowTextureDistance = 100.f;

   mAlignOnTerrain = false;
   mTerrainZOffset = 0;

   mTerrain = NULL;

   mDNStartTime = -1;
   mDNEndTime = -1;


   F32 lMin = 0.5f; //orig: 0.f
   F32 lMax = 1.0f; //orig: 25.5f;

   for (U32 i = 0; i < 162; i++)
   {
      mVelocities[i].x = mRandF(lMin, lMax);
      mVelocities[i].y = mRandF(lMin, lMax);
   }

}



TSDynamic::~TSDynamic()
{
}

//--------------------------------------------------------------------------
void TSDynamic::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Media");
   addField("shapeName", TypeShapeFilename, Offset(mShapeName, TSDynamic));
   addField("animation", TypeCaseString, Offset(mAnimation, TSDynamic));
   addField("radiusX", TypeF32, Offset(mRadiusX, TSDynamic));
   addField("radiusY", TypeF32, Offset(mRadiusY, TSDynamic));
   addField("radiusZ", TypeF32, Offset(mRadiusZ, TSDynamic));
   addField("AlignOnTerrain", TypeBool, Offset(mAlignOnTerrain, TSDynamic));
   addField("TerrainZOffset", TypeF32, Offset(mTerrainZOffset, TSDynamic));
   
   addField("speed", TypeF32, Offset(mSpeed, TSDynamic));
   addField("count", TypeS32, Offset(mCount, TSDynamic));
   addField("myscale", TypeF32, Offset(mMyScale, TSDynamic)); //because changing scale was messing with frustrim clipping
   addField("FadeOutDistance", TypeF32, Offset(mFadeOutDistance, TSDynamic));
   addField("LowTextureDistance", TypeF32, Offset(mLowTextureDistance, TSDynamic));
   endGroup("Media");

   addGroup("Celestials");
     addField("StartHour", TypeS32, Offset(	mDNStartTime, TSDynamic));
	 addField("EndHour", TypeS32, Offset(	mDNEndTime, TSDynamic));
   endGroup("Celestials");


}
//--------------------------------------------------------------------------
void TSDynamic::createShape()
{
   if (!isClientObject())
      return;
   if (!mShapeName || mShapeName[0] == '\0')
      return;

   // If this is the same shape then no reason to update it
   String testShapeName = String(mShapeName);
   if (mShapeInstance && testShapeName.equal(mShape.getPath().getFullPath(), String::NoCase))
       return;

   // Clean up our previous shape
   if (mShapeInstance) {
      SAFE_DELETE(mShapeInstance);
   }
   //reset mThread
   mThread = NULL;

//CRASH?!   if (mShape)
//CRASH?!      SAFE_DELETE(mShape)


   // Attempt to get the resource from the ResourceManager
   mShape = ResourceManager::get().load(mShapeName);

   if (bool(mShape) == false)
   {
      Con::errorf("RenderShapeExample::createShape() - Unable to load shape: %s", mShapeName);
 
      return;
   }

   // Attempt to preload the Materials for this shape
   if (isClientObject() &&
      !mShape->preloadMaterialList(mShape.getPath()) &&
      NetConnection::filesWereDownloaded())
   {
      // NO!!!! mShape = NULL;
      return;
   }

   // Update the bounding box
   updateWorldBox();
   setRenderTransform(mObjToWorld);

   // Create the TSShapeInstance

   mShapeInstance = new TSShapeInstance(mShape, isClientObject());

}
//--------------------------------------------------------------------------
bool TSDynamic::onAdd()
{
   if(!Parent::onAdd())
      return false;

   U32 lMaxCount = Con::getIntVariable("$pref::TS::TSDynamicMaxCount",200);
   if (lMaxCount < 1 ) 
		return false;
 
#ifdef OGE_ENVCACHE
         if (getIsEnvCacheClientObject()) {
            
      		mNetFlags.set(IsGhost);
      		// add to client side mission cleanup
      		SimGroup *cleanup = dynamic_cast<SimGroup *>( Sim::findObject( "ClientMissionCleanup") );
      		if( cleanup != NULL )
      		{
      			cleanup->addObject( this );
      		}
      		else
      		{
      			AssertFatal( false, "Error, could not find ClientMissionCleanup group" );
      			return false;
      		}

         }
#endif


   if (mCount > 162)
	   mCount = 162;

   if ( mCount > lMaxCount  ) 
		mCount = lMaxCount;

   addToScene();

   // Setup the shape.
   if (isClientObject())
   {
      createShape();
      updateAnimation();
   }

   return true;
}
//--------------------------------------------------------------------------
void TSDynamic::updateWorldBox()
{

   const F32 addBoxRad = 10.f;
   if (bool(mShape))
   {

      mObjBox = mShape->mBounds; //bounds;
      if (mRadiusX> 1.f)
      {
         mObjBox.minExtents.x=-mRadiusX* addBoxRad;
         mObjBox.maxExtents.x=mRadiusX * addBoxRad;
      }
      if (mRadiusY> 1.f)
      {
         mObjBox.minExtents.y=-mRadiusY * addBoxRad;
         mObjBox.maxExtents.y=mRadiusY * addBoxRad;
      }
      if (mRadiusZ> 1.f)
      {
         mObjBox.minExtents.z=-mRadiusZ * addBoxRad;
         mObjBox.maxExtents.z=mRadiusZ * addBoxRad;
      }

   }
      
   
   resetWorldBox();
}
//--------------------------------------------------------------------------
void TSDynamic::updateAnimation()
{
   
   if (isClientObject() && mAnimation && mAnimation[0] && mShapeInstance && bool(mShape))
   {

      S32 seq = mShape->findSequence(mAnimation);
      if(seq!=-1)
      {      
         if (!mThread)
         {
            mThread = mShapeInstance->addThread();
         }
         AssertISV(mThread,"Unable to create tsDynamic thread");
         mShapeInstance->setSequence(mThread,seq,0);
      }
   }

}

//--------------------------------------------------------------------------
void TSDynamic::onRemove()
{

   removeFromScene();

   delete mShapeInstance;
   mShapeInstance = NULL;

   Parent::onRemove();
}



//--------------------------------------------------------------------------
/*
*/

void TSDynamic::prepRenderImage(SceneRenderState* state)
{
   //FIXME: shadow && reflection not working so no need to pass it here 

   if (!state->isDiffusePass())
   {
      return;
   }
      

   if (!isRenderEnabled())
      return; 

   // Make sure we have a TSShapeInstance
   if (!mShapeInstance)
      return;

   // Calculate the distance of this object from the camera
   Point3F cameraOffset;
   getRenderTransform().getColumn(3, &cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   //check visible distance .. 
   F32 lCap = mFadeOutDistance + getWorldSphere().radius;
   if (dist > lCap)
      return;


   // Set up the LOD for the shape
   F32 invScale = (1.0f / getMax(getMax(mObjScale.x, mObjScale.y), mObjScale.z));

   mShapeInstance->setDetailFromDistance(state, dist * invScale);

   // Make sure we have a valid level of detail
   if (mShapeInstance->getCurrentDetail() < 0)
      return;
   

   renderObject(state);
}
//--------------------------------------------------------------------------
void TSDynamic::renderObject(SceneRenderState* state)
{

   
   if (mLastTime==0xFFFFFFFF)
      mLastTime = Platform::getVirtualMilliseconds();

   U32 dtime = Platform::getVirtualMilliseconds()-mLastTime;
   mLastTime = Platform::getVirtualMilliseconds();
   F32 dt = F32(dtime)/1000.f;
   F32 time = F32(mLastTime)*.001f;

   if (dt > .1f)
      dt = .1f;

   Point3F location = getPosition();
   MatrixF transform;

   GFXTransformSaver saver;

   // Set up our TS render state      
   TSRenderState rdata;
   rdata.setSceneState(state);
   rdata.setFadeOverride(1.0f);
   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init(getWorldSphere());
   rdata.setLightQuery(&query);

   
   for (U32 i=0;i<mCount;i++)
   {
      if (state->isDiffusePass())
      {
         move(i, time, location, transform);
         mLastTransform[i] = transform;

      } else {
         //FIXME shadow and reflection does not work worldbox problem I guess!!!
         // shadow and reflection need connection to server ^^ 
         transform = mLastTransform[i];
      }


      Point3F cameraOffset = transform.getPosition();
      cameraOffset -= state->getCameraPosition();
      F32 dist = cameraOffset.len();

     if (dist > mFadeOutDistance ) //default distance 150m!
        continue;

     if (mThread)
      mShapeInstance->setTime(mThread,F32(i)*(mShapeInstance->getDuration(mThread)/10.f)+time);

     // Set the world matrix to the objects render transform
     const Point3F lScale = Point3F(mMyScale, mMyScale, mMyScale);
     transform.scale(lScale);
     GFX->setWorldMatrix(transform);

     // Animate the the shape
     mShapeInstance->animate();

     // Allow the shape to submit the RenderInst(s) for itself
     mShapeInstance->render(rdata);


//TODO Low Texture
//     if (dist > mLowTextureDistance) //default 100.f
//        TextureManager::setSmallTexturesActive(true);
//     else
//        TextureManager::setSmallTexturesActive(false);




   } //for each


}


//--------------------------------------------------------------------------

void TSDynamic::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   setRenderTransform(mat);
}

//--------------------------------------------------------------------------

void TSDynamic::move(U32 i, F32 time, const Point3F& org, MatrixF& transform)
{
   if (i>161)
      return;


   // calculate rotation 
   time*=mSpeed;
   F32 angle = time * mVelocities[i][0];
   F32 sy = sin(angle);
   F32 cy = cos(angle);
   angle = time * mVelocities[i][1];
   F32 sp = sin(angle);
   F32 cp = cos(angle);
   Point3F vec;

   vec[0] = cp*cy;
   vec[1] = cp*sy;
   vec[2] = -sp;

   F32 d = sin(time + i);

   
   Point3F loc; // location

   loc[0] = org[0] + gDirs[i][0]*d*mRadiusX + vec[0]*mRadiusX;
   loc[1] = org[1] + gDirs[i][1]*d*mRadiusY + vec[1]*mRadiusY;

   if (mAlignOnTerrain)
   {
      if (mTerrain == NULL) //speed via caching :P
         mTerrain = AIMath::getTerrainBlockATpos(org, false);
	   loc[2] = 0; 
	   loc[2] = AIMath::getTerrainHeight( loc, mTerrain ) + mTerrainZOffset;
   } else {
		loc[2] = org[2] + gDirs[i][2]*d*mRadiusZ + vec[2]*mRadiusZ;
   }

   Point3F n = loc-mLastTransform[i].getPosition();
   n[2]=0.f;
   n.normalize();


   transform = MathUtils::createOrientFromDir(n);

/*
Matrix:
  0 : rightVec.x
  1 : forwardVec.x
  2 : upVec.x
  3 : position.x
  4 : rightVec.y
  5 : forwardVec.z
  6 : upVec.y
  7 : position.y
  8 : rightVec.z
  9 : forwardVec.y
 10 : upVec.z
 11 : position.z
 12 :
 13 :
 14 :
 15 :
 */

   transform.setPosition(loc);

  
}


//--------------------------------------------------------------------------
U32 TSDynamic::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (mCount > 162)
	   mCount = 162;

   mathWrite(*stream, getTransform());
   mathWrite(*stream, getScale());


   stream->write(mDNStartTime);
   stream->write(mDNEndTime);

   stream->write(mCount);
   stream->write(mRadiusX);
   stream->write(mRadiusY);
   stream->write(mRadiusZ);
   stream->write(mSpeed);
   stream->write(mMyScale);

//XXTH terrain align
   stream->write(mAlignOnTerrain);
   stream->write(mTerrainZOffset);


   stream->write(mFadeOutDistance);
   stream->write(mLowTextureDistance);

   stream->writeString(mShapeName);
   stream->writeString(mAnimation);


   return retMask;
}
//--------------------------------------------------------------------------
void TSDynamic::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   MatrixF mat;
   Point3F scale;
   mathRead(*stream, &mat);
   mathRead(*stream, &scale);
   setScale(scale);
   setTransform(mat);

   stream->read(&mDNStartTime);
   stream->read(&mDNEndTime);
   stream->read(&mCount);
   stream->read(&mRadiusX);
   stream->read(&mRadiusY);
   stream->read(&mRadiusZ);
   stream->read(&mSpeed);
   stream->read(&mMyScale);

//XXTH terrain align
   stream->read(&mAlignOnTerrain);
   stream->read(&mTerrainZOffset);


   stream->read(&mFadeOutDistance);
   stream->read(&mLowTextureDistance);


   mShapeName = stream->readSTString();
   mAnimation = stream->readSTString();

  createShape();
  updateAnimation();
  updateWorldBox();

}
//--------------------------------------------------------------------------
void TSDynamic::inspectPostApply()
{
   if(isServerObject()) {
      setMaskBits(0xffffffff);
   } 
   if (mCount > 162)
	   mCount = 162;
   updateWorldBox();
}
