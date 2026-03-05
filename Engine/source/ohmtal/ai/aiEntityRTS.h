//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// AIEntityRTS
// @author T.Huehn (XXTH)
// @desc need less collisions and some small modifications
//
// * Bases on AIentity
// * NO Collisions
// * Drop to Terrain
// * Animations
// * Movement
// * Callbacks against Object NOT datablock
//-----------------------------------------------------------------------------

#ifndef _AIENTITYRTS_H_
#define _AIENTITYRTS_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

#ifndef _AIENTITY_H_
#include "aiEntity.h"
#endif



class AIEntityRTS : public AIEntity 
{
protected:
   typedef AIEntity Parent;
   
public:
   DECLARE_CONOBJECT(AIEntityRTS);

   AIEntityRTS() {
      mCollisionMoveMask = (
         TerrainObjectType |
         TerrainLikeObjectType |
         InteriorLikeObjectType |
         WaterObjectType |
         // PlayerObjectType     | 
         // StaticShapeObjectType |
         VehicleObjectType |
         PhysicalZoneObjectType);



      mServerCollisionContactMask = (mCollisionMoveMask |
         (ItemObjectType |
            TriggerObjectType |
            CorpseObjectType
            ));

      mClientCollisionContactMask = mCollisionMoveMask | PhysicalZoneObjectType;

   }
   
   void processTick(const Move* move);
   void updateMove(const Move* move);
   bool updatePos(const F32 travelTime = TickSec);
   void dropToTerrain(Point3F& location);
   void dropToGround(Point3F& location);

   // Utility Methods
   virtual void throwCallback(const char* name);
};

#endif //_AIENTITYRTS_H_
