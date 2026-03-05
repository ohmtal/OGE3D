//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _TERRAINGRID_H_
#define _TERRAINGRID_H_

#ifndef _BASICGRID_H_
#include "basicgrid.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

class TerrainGrid : public SceneObject
{
   typedef SceneObject Parent;

   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0,
      GridMask     = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
   };

protected:
   BasicGrid* mGrid;
   RectI mArea;
   F32 mSquareSize;
   TerrainBlock* mTerrain;
   bool mVisible;

   S32 mFaction; //grid have a faction :)

/*
   NetStringHandle   mBlockedMaterialName1;
   NetStringHandle   mBlockedMaterialName2;
   NetStringHandle   mBlockedMaterialName2;
*/
   //rendering stuff
   GFXStateBlockDesc mRectFillDesc;
   S32 mGuessedPlaneCount;
   S32    mRenderAlpha;
   ColorI mRenderBaseColor;

   
   


public:
   DECLARE_CONOBJECT(TerrainGrid);
   TerrainGrid();
   ~TerrainGrid();
   static void initPersistFields();
   bool onAdd();
   void onRemove();
   void updateWorldBox();
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* conn, BitStream* stream);
   virtual bool init();
   void prepRenderImage(SceneRenderState* state);

   void renderSimple(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* mi);

   void setVisible(bool lValue) { mVisible = lValue; }
   bool getVisible() { return mVisible; }

   BasicGrid* getGrid() { return mGrid; }
   bool addFlag(F32 x, F32 y, U8 lFlagId);

   const F32 cRenderDist = 200.f;

};

#endif //_TERRAINGRID_H_
