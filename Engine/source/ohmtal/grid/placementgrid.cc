//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include "T3D/missionArea.h"
#include "basicgrid.h"
#include "terraingrid.h"
#include "ohmtal/ai/aiMath.h"

#include "placementgrid.h"

IMPLEMENT_CO_NETOBJECT_V1(PlacementGrid);
//-----------------------------------------------------------------------------
PlacementGrid::PlacementGrid()
{
   mZ = 0.f;
   mVisible = true;
   mSquareSize = 8.f; 
}
//-----------------------------------------------------------------------------
void PlacementGrid::setZ(F32 lZ)
{
   if (lZ == mZ)
      return;
   mZ = lZ;
   BasicGridNode* lNode;
   Point3F lPos;
   for (S32 i = 0; i < mGrid->getNodeCount(); i++)
   {
      lNode = mGrid->getNodeById(i);
      lPos = lNode->getPos();
      lPos.z = mZ;
      lNode->setPos(lPos);
   }


}
//-----------------------------------------------------------------------------
bool PlacementGrid::castRay(const Point3F& start, const Point3F& end, RayInfo* info)
{
   //   PlaneF plane( Point3F( 0.0f, 0.0f, 0.0f ), Point3F( 0.0f, 0.0f, 1.0f ) );
   PlaneF plane(Point3F(0.0f, 0.0f, mZ), Point3F(0.0f, 0.0f, 1.0f));

   F32 t = plane.intersect(start, end);
   if (t >= 0.0 && t <= 1.0)
   {
      info->t = t;

      Point3F startToEnd = end - start;
      startToEnd *= t;
      info->point = startToEnd + start;

      info->normal.set(0, 0, 1);
      info->object = this;
      info->distance = 0;
      info->faceDot = 0;
      return true;
   }

   return false;
}
//-----------------------------------------------------------------------------
bool PlacementGrid::init()
{
   if (mSquareSize == 0)
   {
      Con::errorf("PlacementGrid::init - Invalid Squaresize! ");
      return false;
   }
   if (mGrid->getInitDone())
      return true;

   RectI lArea;
   if (mArea.isValidRect())
   {
      lArea = mArea;
   }
   else {
      MissionArea* missionAreaPtr = MissionArea::getServerObject();
      if (!missionAreaPtr)
      {
         // return false;
         lArea = MissionArea::smMissionArea;
         Con::warnf("MissionArea does not exists!!!!! ");
      }
      else {
         lArea = missionAreaPtr->getArea();
      }
   }

   if (!lArea.isValidRect())
      return false;


   Point2I lAreaCenter = lArea.point + lArea.extent / 2;

   mGrid->init(lArea, mSquareSize);

   setZ(mZ);

   // set the shit clean it's done on server AND client
   mGrid->setClean();
   if (isClientObject())
   {
      mGuessedPlaneCount = (S32)(cRenderDist * cRenderDist / mGrid->getSquareSize() / 2.f);
#ifdef TORQUE_DEBUG
      mGrid->PrintInfo();
#endif
   }

   updateWorldBox();

   addToScene();

   return true;

}
//-----------------------------------------------------------------------------
/*
ConsoleFunction(PlacementGrid_Create, S32, 1, 2, "Create a PlacementGrid optional param F32 SquareSize")
{
   PlacementGrid* lPlacementGrid = new PlacementGrid();

   F32 lSquareSize;
   if (argc > 1)
      lPlacementGrid->setSquareSize(dAtof(argv[1]));


   lPlacementGrid->init();
   if (!lPlacementGrid->registerObject())
   {
      Con::errorf("Failed to register PlacementGrid!");
      SAFE_DELETE(lPlacementGrid);
      return 0;
   }
   return lPlacementGrid->getId();
}
*/

DefineEngineMethod(PlacementGrid, setZ, void, (F32 z), , "float z")
{
   object->setZ(z);
}

DefineEngineMethod(PlacementGrid, getZ, F32,(), , "get Z position of PlacementGrid")
{
   return object->getZ();
}
