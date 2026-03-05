//-----------------------------------------------------------------------------
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

#ifndef _TSDynamic_H_
#define _TSDynamic_H_


#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif

/*
#ifndef _SCENEOBJECT_H_
#include "sim/sceneObject.h"
#endif
#ifndef _RESMANAGER_H_
#include "core/resManager.h"
#endif
*/

#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif


class TSThread;
class TSShape;
class TSShapeInstance;
class TSDynamic;



//--------------------------------------------------------------------------
class TSDynamic : public SceneObject
{
   typedef SceneObject Parent;
   static U32 smUniqueIdentifier;


  protected:
   bool onAdd();
   void onRemove();

  protected:

   StringTableEntry  mAnimation;
   StringTableEntry  mShapeName;
   F32               mRadiusX;
   F32               mRadiusY;
   F32               mRadiusZ;
   F32               mSpeed;
   S32               mCount;
   F32               mMyScale;
   Resource<TSShape> mShape;
   TSShapeInstance*  mShapeInstance;
   U32               mLastTime;
   TSThread*         mThread;
   Point2F           mVelocities[162];
   MatrixF           mLastTransform[162];

   F32               mFadeOutDistance;
   F32               mLowTextureDistance;

   bool				mAlignOnTerrain;
   F32				mTerrainZOffset;
   TerrainBlock * mTerrain;


  public:
   S32				mDNStartTime;
   S32				mDNEndTime;
   

   // Rendering
  protected:


   virtual void prepRenderImage(SceneRenderState* state);
   void renderObject ( SceneRenderState *state);   
   void setTransform     ( const MatrixF &mat);
   void move(U32 i, F32 time, const Point3F& org, MatrixF& transform);
   void updateAnimation();
   void updateWorldBox();
   //S32 rand();

   // Create the geometry for rendering
   void createShape();


  public:
   TSDynamic();
   ~TSDynamic();

   DECLARE_CONOBJECT(TSDynamic);
   static void initPersistFields();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);

   void inspectPostApply();



};

#endif // _TSDynamic_H_ 

