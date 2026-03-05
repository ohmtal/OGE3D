/**
  tom2DGridRender.h

  @since 2024-01-04
  @author XXTH

  License at: ohmtal/misc/ohmtalMIT.h

*/
#ifndef _TOM2DGRIDRENDER_H_
#define _TOM2DGRIDRENDER_H_

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

#ifndef _BASICGRID_H_
#include "ohmtal/grid/basicgrid.h"
#endif

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif




class tom2DGridRender : public tom2DRenderObject
{
private:
   typedef tom2DRenderObject Parent;


public:
   //creation methods
   DECLARE_CONOBJECT(tom2DGridRender);
   tom2DGridRender();
   ~tom2DGridRender();

protected:
   BasicGrid* mGrid;
   U32 mTriangleCount; //count of the mTriangleCount in the vertex Buffer
   GFXVertexBufferHandle<GFXVertexPC> mVertBuff;

   ColorI mColorTable[256];

public:
   void setGrid(BasicGrid* lGrid);
   BasicGrid* getGrid() { return mGrid; };
   void updateVerts();

   virtual void onRender(U32 dt, Point3F lOffset);

   ColorI  getWeightColor(U8 index) { return mColorTable[index]; }
   void setWeightColor(U8 index, ColorI lColor);

};
#endif //_TOM2DGRIDRENDER_H_
