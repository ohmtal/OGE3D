//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include <terrain/terrData.h>


namespace OhmtalTerrainTools
{
   //use AIMath::getTerrainBlockATpos(location, true); !!!
/*
   TerrainBlock* getTerrainUnderWorldPoint(const Point3F& wPos)
   {
      // Cast a ray straight down from the world position and see which
      // Terrain is the closest to our starting point
      Point3F startPnt = wPos;
      Point3F endPnt = wPos + Point3F(0.0f, 0.0f, -10000.0f);

      S32 blockIndex = -1;
      F32 nearT = 1.0f;

      SimpleQueryList queryList;
      gServerContainer.findObjects(TerrainObjectType, SimpleQueryList::insertionCallback, &queryList);

      for (U32 i = 0; i < queryList.mList.size(); i++)
      {
         Point3F tStartPnt, tEndPnt;
         TerrainBlock* terrBlock = dynamic_cast<TerrainBlock*>(queryList.mList[i]);
         terrBlock->getWorldTransform().mulP(startPnt, &tStartPnt);
         terrBlock->getWorldTransform().mulP(endPnt, &tEndPnt);

         RayInfo ri;
         if (terrBlock->castRayI(tStartPnt, tEndPnt, &ri, true))
         {
            if (ri.t < nearT)
            {
               blockIndex = i;
               nearT = ri.t;
            }
         }
      }

      if (blockIndex > -1)
         return (TerrainBlock*)(queryList.mList[blockIndex]);

      return NULL;
   }
*/

   

}
