//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _PLACEMENTGRID_H_
#define _PLACEMENTGRID_H_

#ifndef _TERRAINGRID_H_
#include "terraingrid.h"
#endif // !_TERRAINGRID_H_

class PlacementGrid : public TerrainGrid
{
   typedef TerrainGrid Parent;
private:
   F32 mZ;
public:
   PlacementGrid();
   virtual bool castRay(const Point3F& start, const Point3F& end, RayInfo* info);
   virtual bool init();
   void setZ(F32 lZ);
   F32 getZ() { return mZ; }

   DECLARE_CONOBJECT(PlacementGrid);
};



#endif //_PLACEMENTGRID_H_
