//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _GFXOBJECT_H_
#define _GFXOBJECT_H_


#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif



#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

class GFXObject : public SceneObject, public ITickable
{
   typedef SceneObject Parent;

   // Networking masks
   enum MaskBits
   {
      UpdateMask    = Parent::NextFreeMask << 0,
      NextFreeMask  = Parent::NextFreeMask << 1
   };


   // StateBlock description 
   GFXStateBlockDesc mStateBlockDesc;

   
public:
   GFXObject();
   virtual ~GFXObject();


   //what can be rendered here ?! 
   enum RenderType
   {
      Cube     = 0,
      Capsule  = 1,
      Sphere   = 2,
      Cylinder = 3,
      Cone     = 4,
      Arc      = 5,
      Arrow    = 6
   };

   enum BlendType
   {
      BLEND_NORMAL      = 0,
      BLEND_ADDITIVE    = 1,
      BLEND_SUBTRACTIVE = 2,
      BLEND_DISABLED = 3,
   };

   // Declare this object as a ConsoleObject so that we can
   // instantiate it into the world and network it
   DECLARE_CONOBJECT(GFXObject);

   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

   void inspectPostApply();

   // Handle when we are added to the scene and removed from the scene
   bool onAdd();
   void onRemove();

   // Override this so that we can dirty the network flag when it is called
   void setTransform(const MatrixF& mat);

   // This function handles sending the relevant data from the server
   // object to the client object
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   // This function handles receiving relevant data from the server
   // object and applying it to the client object
   void unpackUpdate(NetConnection* conn, BitStream* stream);



   //--------------------------------------------------------------------------
   // Object Rendering
   //--------------------------------------------------------------------------

   // This is the function that allows this object to submit itself for rendering
   void prepRenderImage(SceneRenderState* state);

   void renderArc();

   // This is the function that actually gets called to do the rendering
   // Note that there is no longer a predefined name for this function.
   // Instead, when we submit our ObjectRenderInst in prepRenderImage(),
   // we bind this function as our rendering delegate function
   void render(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat);

protected:
   ColorI mColor;
   F32    mRadius;
   F32    mHeight;
   GFXFillMode mFillMode;
   RenderType mRenderType;
   BlendType   mBlendType;
   U32 mArcAngle;
   Point3F mPoint;

   void updateWorldBox();
   void updateStateBlockDesc();

   //iTickable interface
   virtual void interpolateTick(F32 dt);
   virtual void processTick();
   virtual void advanceTime(F32 dt);


}; //class

typedef GFXObject::RenderType   GFXObjectRenderType;
DefineEnumType(GFXObjectRenderType);

typedef GFXFillMode GFXObjectFillMode;
DefineEnumType(GFXObjectFillMode);

typedef GFXObject::BlendType GFXObjectBlendType;
DefineEnumType(GFXObjectBlendType);


#endif //_GFXOBJECT_H_
