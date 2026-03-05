/**
 * TerrainTweaker
 * set TerrainTexture by script

*/

#include "gui/worldEditor/terrainEditor.h"


/*
void TerrainEditor::setGridInfo(const GridInfo & info, bool checkActive)
{
   PROFILE_SCOPE( TerrainEditor_SetGridInfo );

   setGridHeight(info.mGridPoint, info.mHeight);
   setGridMaterial(info.mGridPoint, info.mMaterial);
}

index it the number of the material index
Gridpoint is a bit more complicated, because we need the terrain block:
struct GridPoint
{
   Point2I        gridPos;
   TerrainBlock*  terrainBlock;

   GridPoint() { gridPos.set(0, 0); terrainBlock = NULL; };
};

const TerrainBlock *terrain = mTerrainEditor->getActiveTerrain();


void TerrainEditor::setGridMaterial( const GridPoint &gPoint, U8 index )
{
   PROFILE_SCOPE( TerrainEditor_SetGridMaterial );

   Point2I cPos;
   gridToCenter( gPoint.gridPos, cPos );
   TerrainFile *file = gPoint.terrainBlock->getFile();

   // If we changed the empty state then we need
   // to do a grid update as well.
   U8 currIndex = file->getLayerIndex( cPos.x, cPos.y );
   if (  ( currIndex == (U8)-1 && index != (U8)-1 ) ||
         ( currIndex != (U8)-1 && index == (U8)-1 ) )
   {
      mGridUpdateMin.setMin( cPos );
      mGridUpdateMax.setMax( cPos );
      mNeedsGridUpdate = true;
   }

   file->setLayerIndex( cPos.x, cPos.y, index );
}


*/


// example DefineEngineMethod( AIEntity, getTargetPlayerType, S32, (), , "()" "Gets Monster target Playertype.")
// Called in editor with: ETerrainEditor.showActiveTerrain();



/*  THIS EXITS WITH : ETerrainEditor.getActiveTerrain(); ==>
*     DefineEngineMethod( TerrainEditor, getActiveTerrain, S32, (), , "")
DefineEngineMethod(TerrainEditor, showActiveTerrain, void, (), , "XXTHTEST show the active terrain ")
{

   const TerrainBlock* terrain = object->getActiveTerrain();
   Con::printf("Active Terrain is: %s", terrain->getIdString());

}
*/

//----- this is what is used by autopaint:
// ETerrainEditor.autoMaterialLayer($TPPHeightMin, $TPPHeightMax, $TPPSlopeMin, $TPPSlopeMax, $TPPCoverage);


// void TerrainEditor::setGridMaterial(const GridPoint& gPoint, U8 index)
// bool TerrainEditor::worldToGrid(const Point3F& wPos, GridPoint& gPoint)

//ETerrainEditor.paintTerrainTest(pl_visitor.getposition(), 0);
DefineEngineMethod(TerrainEditor, paintTerrainTest, bool, (Point3F worldPos, U8 materialIndex), , "XXTHTEST paint on the terrain ")
{
   GridPoint myPoint;
   if (!object->worldToGrid(worldPos, myPoint))
      return false;
   object->setGridMaterial(myPoint, materialIndex);

   //without this it SHOULD only show after reload - but it does NOT something is missing mhhh
   // when i then manually paint the change show up...
   object->scheduleMaterialUpdate();

   return true;
   
}
// THIS WORLD PERFECT -- note editor should be loaded once else worldtoGrid faild
//ETerrainEditor.paintTerrainAtPos(pl_visitor.getposition(), 0);
DefineEngineMethod(TerrainEditor, paintTerrainAtPos, bool, (Point3F worldPos, U8 materialIndex),
   , "paint on the terrain\n"
   "NOTE: editor should be loaded once else worldtoGrid fail\n "
   "NOTE: TerrainPainter should be active to see the new painting.\n "
)
{
   GridPoint myPoint;
   if (!object->worldToGrid(worldPos, myPoint))
      return false;

   GridInfo info; // the height at the brush position
   object->getGridInfo(myPoint.gridPos, info, myPoint.terrainBlock);

   info.mMaterial = materialIndex;
   object->setGridInfo(info);
   object->scheduleMaterialUpdate();
   return true;
}


