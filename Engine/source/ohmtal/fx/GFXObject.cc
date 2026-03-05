#include "GFXObject.h"
//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
/**

Problem: Color Alpha also change the color ?! 



 class  GFXObject
 @author T.Huehn 2023 (XXTH)



 GFXStateBlockDesc
=================
	Like: 
	   mSelectedCubeDesc.setZReadWrite(true, false);
	   mSelectedCubeDesc.setBlend(true);
	   mSelectedCubeDesc.fillMode = GFXFillWireframe;
    

	FillMode
	--------
		enum GFXFillMode 
		{
		   GFXFill_FIRST = 1,
		   GFXFillPoint = 1,
		   GFXFillWireframe,
		   GFXFillSolid,
		   GFXFill_COUNT
		};

GFXDrawUtil
===========
	Like: 
      Box3F lBox = mSelectedObj->getWorldBox();
      GFX->getDrawUtil()->drawCube(mSelectedCubeDesc, lBox, ColorI(230, 250, 60, 255));
*/



#include "GFXObject.h"

#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "materials/sceneData.h"
#include "renderInstance/renderPassManager.h"
#include "console/console.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxDevice.h"
#include "gfx/primBuilder.h"




IMPLEMENT_CO_NETOBJECT_V1(GFXObject);

ConsoleDocClass(GFXObject,
   "@brief An Object where primitives can be rendered\n\n"
   "At the moment it's unfinished and only use render wireframe box  "
   "@ingroup Ohmtal\n");

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ImplementEnumType(GFXObjectRenderType,
   "Type of the GFXObject rendering.\n"
   "@ingroup gameObjects")
{ GFXObject::Cube, "Cube", "Cube using Worldbox" },
{ GFXObject::Capsule, "Capsule", "Capsule using Radius and Height" },
{ GFXObject::Sphere, "Sphere", "Sphere using Radius" },
{ GFXObject::Cylinder, "Cylinder", "using Radius and Height" },
{ GFXObject::Cone, "Cone", "using Radius and Height" },
{ GFXObject::Arc, "Arc", "animated arc using Radius" },
{ GFXObject::Arrow, "Arrow", "using Point" },
EndImplementEnumType;

//-----------------------------------------------------------------------------
ImplementEnumType(GFXObjectFillMode,
   "FillMode.\n"
   "@ingroup gameObjects")
{ GFXFillWireframe, "WireFrame", "" },
{ GFXFillSolid, "Solid", "" },
EndImplementEnumType;
//-----------------------------------------------------------------------------
ImplementEnumType(GFXObjectBlendType,
   "Blend type.\n"
   "@ingroup gameObjects")
{ GFXObject::BLEND_DISABLED,     "Disabled", "" },
{ GFXObject::BLEND_NORMAL,       "Normal", "" },
{ GFXObject::BLEND_ADDITIVE,     "Additive", "" },
{ GFXObject::BLEND_SUBTRACTIVE,  "Subtractive", "" },
EndImplementEnumType;



//-----------------------------------------------------------------------------
// constructor
GFXObject::GFXObject()
{
   // FIXME ScopeAlways ?! 
   mNetFlags.set(Ghostable); // | ScopeAlways);

   // Set it as a "static" object
   mTypeMask |= StaticObjectType | StaticShapeObjectType;




   //base color
   mRenderType = Cube;
   mColor  = ColorI(230, 250, 60, 255);
   mRadius = 1.f;
   mHeight = 1.f;
   mFillMode = GFXFillSolid;
   mPoint.zero();
   mBlendType = BLEND_NORMAL;


   
   //setup the description 
//XXTH TEST    mStateBlockDesc.setZReadWrite(true,true);
   mStateBlockDesc.setZReadWrite(true,false);
   mStateBlockDesc.setCullMode(GFXCullNone);
   

    updateStateBlockDesc(); //set fillmode and bendtype

}
//-----------------------------------------------------------------------------
//Destructor
GFXObject::~GFXObject()
{
}

//-----------------------------------------------------------------------------
//Script properties 
void GFXObject::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("GFXObject");

   addField("RenderType", TypeGFXObjectRenderType,Offset(mRenderType, GFXObject),
         "The type we want to render"
      );

   addField("FillMode", TypeGFXObjectFillMode, Offset(mFillMode, GFXObject),
      "The fillmode"
   );
   addField("BlendType", TypeGFXObjectBlendType, Offset(mBlendType, GFXObject),
      "Blend type."
   );
   


      addField("color", TypeColorI, Offset(mColor, GFXObject), "Render Color");
      addField("radius", TypeF32, Offset(mRadius, GFXObject), "Render Radius ");
      addField("height", TypeF32, Offset(mHeight, GFXObject), "Render Height ");
      addField("tipPoint", TypePoint3F, Offset(mPoint, GFXObject), "Render extra Point (e.g. for arrow) ");
      

   endGroup("GFXObject");
}

void GFXObject::inspectPostApply()
{
   setMaskBits(UpdateMask);
   // if (getIsEnvCacheClientObject())
   {
      updateWorldBox();
   }
      

}

//-----------------------------------------------------------------------------
//Object is added on client or server 
bool GFXObject::onAdd()
{
   if (!Parent::onAdd())
      return false;

   updateWorldBox();

   // Add this object to the scene
   addToScene();

   return true;
}
//-----------------------------------------------------------------------------
//Object is removed on client or server 
void GFXObject::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   Parent::onRemove();

}

//-----------------------------------------------------------------------------
//set our transform (position and rotation)
void GFXObject::setTransform(const MatrixF& mat)
{
   // Let SceneObject handle all of the matrix manipulation
   Parent::setTransform(mat);

   // Dirty our network mask so that the new transform gets
   // transmitted to the client object
   setMaskBits(UpdateMask);

}

//-----------------------------------------------------------------------------
//on server we pack the update for the client ...
U32 GFXObject::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);


   // Write our transform information
   if (stream->writeFlag(mask & UpdateMask))
   {
      stream->write((U32)mRenderType);
      stream->write((U32)mFillMode);
      stream->write((U32)mBlendType);
      
      stream->write(mColor);
      stream->write(mRadius);
      stream->write(mHeight);
      mathWrite(*stream, mPoint);
      

      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   return retMask;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//on client we unpack the package we received.
void GFXObject::unpackUpdate(NetConnection* conn, BitStream* stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())  // UpdateMask
   {
      U32 lRenderType;
      stream->read(&lRenderType);
      mRenderType = (RenderType) lRenderType;

      U32 lFillMode;
      stream->read(&lFillMode);
      mFillMode = (GFXFillMode)lFillMode;

      U32 lBlendType;
      stream->read(&lBlendType);
      mBlendType = (BlendType)lBlendType;


      updateStateBlockDesc(); //set fillmode and bendtype

      stream->read(&mColor);
      stream->read(&mRadius);
      stream->read(&mHeight);
      mathRead(*stream, &mPoint);

      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);

      updateWorldBox();
   }

}

//-----------------------------------------------------------------------------
//on client add to render instance
void GFXObject::prepRenderImage(SceneRenderState* state)
{

   // Allocate an ObjectRenderInst so that we can submit it to the RenderPassManager
   ObjectRenderInst* ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   // Now bind our rendering function so that it will get called
   ri->renderDelegate.bind(this, &GFXObject::render);

   // Set our RenderInst as a standard object render
   ri->type = RenderPassManager::RIT_Object;
   
   


   // Set our sorting keys to a default value
   ri->defaultKey = 0; 
   ri->defaultKey2 = 0;

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst(ri);

}
//-----------------------------------------------------------------------------
//
// mCreationAreaAngle = (U32)(mCreationAreaAngle + (1000 * ElapsedTime));
// mCreationAreaAngle = mCreationAreaAngle % 360;
// 
// Renders a triangle stripped oval

/*

*/

void GFXObject::renderArc()
{
   GFX->pushWorldMatrix();
   GFX->multWorld(getRenderTransform());
   //FIXME stateblock!! 
   GFXStateBlockRef lFixmeStateBlock;
   lFixmeStateBlock = GFX->createStateBlock(mStateBlockDesc);
   GFX->setStateBlock(lFixmeStateBlock);
   //~~~~~~~
   PrimBuild::begin(GFXTriangleStrip, 720);
   for (U32 Angle = mArcAngle; Angle < (mArcAngle + 360); Angle++)
   {
      F32 XPos, YPos;
      U32 lAlpha;

      // Calculate Position.
      XPos = mRadius * mCos(mDegToRad(-(F32)Angle));
      YPos = mRadius * mSin(mDegToRad(-(F32)Angle));


      //lAlpha = Angle - mArcAngle;
      // lAlpha = (U32)mFloor((Angle - mArcAngle) / 2.8125f) + 128;

      //(0..255)
      lAlpha = (U32)mFloor((Angle - mArcAngle) / 1.40625f);
       
      // Set Colour.
      PrimBuild::color4i(mColor.red, mColor.green, mColor.blue, lAlpha);

      PrimBuild::vertex3f(XPos, YPos, -(F32)mHeight / 2.0f);
      PrimBuild::vertex3f(XPos, YPos, +(F32)mHeight / 2.0f);
   }
   PrimBuild::end();
   //~~~~~~~
   GFX->popWorldMatrix();
}

//-----------------------------------------------------------------------------
//render the stuff 
void GFXObject::render(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat)
{
   if (!isRenderEnabled())
      return;
   Point3F lTipPnt; //used for cylinder, cone and arrow

   switch (mRenderType)
   {
      case GFXObject::Cube:
         GFX->getDrawUtil()->drawCube(mStateBlockDesc, getRenderWorldBox(), mColor);
         break;
      case GFXObject::Capsule:
         GFX->getDrawUtil()->drawCapsule(mStateBlockDesc, getRenderPosition(), mRadius, mHeight, mColor);
         break;
      case GFXObject::Sphere:
         GFX->getDrawUtil()->drawSphere(mStateBlockDesc, mRadius, getRenderPosition(), mColor, true, true);
         break;
      case GFXObject::Cylinder:
         if (mPoint.isZero())
            lTipPnt = getRenderPosition() + Point3F(0.f, 0.f, mHeight);
         else
            lTipPnt = mPoint;

         GFX->getDrawUtil()->drawCylinder(mStateBlockDesc, getRenderPosition(), lTipPnt, mRadius, mColor);
         break;

      case GFXObject::Cone:
         if (mPoint.isZero())
            lTipPnt = getRenderPosition() + Point3F(0.f, 0.f, mHeight);
         else
            lTipPnt = mPoint;

         GFX->getDrawUtil()->drawCone(mStateBlockDesc, getRenderPosition(), lTipPnt, mRadius, mColor);
         break;

      case GFXObject::Arc:
         renderArc();
         break;


      case GFXObject::Arrow:
         // void drawArrow(const GFXStateBlockDesc & desc, const Point3F & start, const Point3F & end, const ColorI & color, F32 baseRad = 0.0f);
         if (mPoint.isZero())
            lTipPnt = getRenderPosition() + Point3F(0.f, 0.f, mHeight);
         else
            lTipPnt = mPoint;
         
         GFX->getDrawUtil()->drawArrow(mStateBlockDesc, getRenderPosition(), lTipPnt, mColor, mRadius - 1.f);
         break;
   }
}
//------------------------------------------------------------------------------
/*
   update the worldbox with the size of the object,
   a bit compilicated because of different shapes and tippoint
   some use scale some radius and height 
*/
void GFXObject::updateWorldBox()
{

   Point3F lTipPnt; //used for cylinder, cone and arrow

   switch (mRenderType)
   {
      case GFXObject::Sphere:
         // using mRadius 
         mObjBox.minExtents.set(-mRadius, -mRadius, -mRadius);
         mObjBox.maxExtents.set(mRadius, mRadius, mRadius);
         break;
      case GFXObject::Capsule:
      case GFXObject::Arc:
         // using mRadius and mHeight
         mObjBox.minExtents.set(-mRadius, -mRadius, -mAbs(mHeight));
         mObjBox.maxExtents.set(mRadius, mRadius, mAbs(mHeight));

         break;
      case GFXObject::Cylinder:
      case GFXObject::Cone:
      case GFXObject::Arrow:
         // using mRadius and mHeight + tipPoint
         if (mPoint.isZero())
         {
            mObjBox.minExtents.set(-mRadius, -mRadius, -mAbs(mHeight));
            mObjBox.maxExtents.set(mRadius, mRadius, mAbs(mHeight));
         }
         else {
            
            Point3F lPoint = getPosition() - mPoint;
            lPoint.x = mAbs(lPoint.x);
            lPoint.y = mAbs(lPoint.y);
            lPoint.z = mAbs(lPoint.z);

            mObjBox.minExtents.set(
                 getMin( -mRadius, -lPoint.x )
               , getMin( -mRadius, -lPoint.y )
               , getMin( -mHeight, -lPoint.z )
            );
            mObjBox.maxExtents.set(
                 getMax( mRadius, lPoint.x )
               , getMax( mRadius, lPoint.y )
               , getMax( mHeight, lPoint.z )
            );
            
         }
         break;

      // default:
      case GFXObject::Cube:
         // simple box using scale 
         mObjBox.minExtents.set(-getScale() * 0.5f);
         mObjBox.maxExtents.set(getScale() * 0.5f);
         break;

   }




   resetWorldBox();
}
//------------------------------------------------------------------------------
void GFXObject::updateStateBlockDesc()
{
   mStateBlockDesc.fillMode = mFillMode;

   switch (mBlendType)
   {
      case BLEND_DISABLED:
         mStateBlockDesc.setBlend(false);
         break;
      case BLEND_ADDITIVE:
         mStateBlockDesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendOne);
      break;
      case BLEND_SUBTRACTIVE:
         mStateBlockDesc.setBlend(true, GFXBlendZero, GFXBlendInvSrcColor);
      break;
      case BLEND_NORMAL:
         mStateBlockDesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
         
      break;
   }
   /*
   mStateBlockDesc.setAlphaTest(true, GFXCmpGreater, 0);

   mStateBlockDesc.setZReadWrite(true);
   mStateBlockDesc.zFunc = GFXCmpLessEqual;
   mStateBlockDesc.zWriteEnable = false;
   mStateBlockDesc.samplersDefined = true;
   mStateBlockDesc.samplers[0].textureColorOp = GFXTOPModulate;
   mStateBlockDesc.samplers[1].textureColorOp = GFXTOPDisable;
   */

}

//------------------------------------------------------------------------------
// iTickable interface
void GFXObject::interpolateTick(F32 dt)
{
   Parent::interpolateTick(dt);
      
}

void GFXObject::processTick()
{
   //XXTH_MOUNT
   if (isMounted()) {
      MatrixF mat;
      mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat);
      Parent::setTransform(mat);
   }
}

void GFXObject::advanceTime(F32 dt)
{
   if (isServerObject())
      return;
   if (mRenderType == GFXObject::Arc)
   {
      mArcAngle += U32(dt * 1000);
      mArcAngle %= 360;

   }
}
