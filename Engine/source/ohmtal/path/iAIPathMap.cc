/*------------------------------------------------------------------ -
The MIT License(MIT)

Copyright(c) 2006 Gavin Bunney and Tom Romano
Copyright(c) 2009/23 huehn-software / Ohmtal Game Studio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
https://github.com/dawogfather/immersiveAI/
-------------------------------------------------------------------*/

#include "T3D/missionArea.h"
#include "platform/profiler.h"
#include "terrain/terrData.h"
#include "T3D/gameBase/gameConnection.h"

#include "iAIPathMap.h"
#include "iAIPathGlobal.h"
#include "iAIPathGrid.h"
#include "iAIPathNode.h"

#include "core/util/safeDelete.h"

IMPLEMENT_CONOBJECT(iAIPathMap);

U32 iAIPathMap::smNodeCount = 0;

iAIPathMap::iAIPathMap()
{
	this->mCompiled = false;
	//this->mTerrainGridIndex = 0;
    this->mTerrainGridObj = NULL;
	this->mMapBitmapHandle = NULL;
}

iAIPathMap::~iAIPathMap()
{
	this->clearMap();
}

bool iAIPathMap::initialize()
{

	// if not compiled, create the path map
	if (!this->mCompiled)
	{
		// create a new pathmap
		this->mCompiled = this->createPathMap();
	}
	return this->mCompiled;
}

//--------------------------------------------------------------------------------------------------------
// XXTH This function is not really optimal, it clears all grids and create a mission area grid, if you 
//      want or not. Finally it calculate the nodecount on the mission area grid only. 
//      Must be changed to create multiple and individual grids


bool iAIPathMap::createPathMap()
{
	//XXTH Con::iAIMessagef("Immersive AI :: Seek :: Building PathMap...");
	Con::printf("Immersive AI :: Seek :: Building PathMap...");
		

	// clear any current path map
	this->clearMap();

	// generate an avoid list of all the grids
	Vector<Box3F> avoidList;
	for (U32 i = 0; i < this->mGrids.size(); ++i)
		avoidList.push_back(this->mGrids[i]->getWorldBox());


	if  (Con::getBoolVariable("$Server::CreateMissionAreaGrid",true)) {
		// calculate the entire mission area
      MissionArea* missionAreaPtr = MissionArea::getServerObject(); //XXTH dynamic_cast<MissionArea*>(Sim::findObject("MissionArea"));
		RectI lArea;
		if (!missionAreaPtr)
		{
			// return false;
			lArea = MissionArea::smMissionArea;

		} else {
			lArea= missionAreaPtr->getArea();
		}
		// set grid points are the initial mission area points in x&y to the extent of the mission area
		Point3F gridStart = Point3F(lArea.point.x, lArea.point.y, -10.0);
		Point3F gridEnd = Point3F(lArea.point.x + lArea.extent.x, lArea.point.y + lArea.extent.y, 1000.0);

		// pointer for the terrain grid
		iAIPathGrid *terrainGrid = new iAIPathGrid();
		this->mTerrainGridObj = terrainGrid;

		// generate a grid for the terrain, avoiding the avoid list
		if (terrainGrid->createTerrainGrid(gridStart, gridEnd, mMapBitmapHandle,  avoidList, IAIPATHGLOBAL_GRID_DENSITY_TERRAIN))
		{
			// add to pathmap collection
			this->mGrids.push_back(terrainGrid);
	
			// set the type of the terrain grid to just a zone type;
			// needed to optimise the getClosestNode function
			// (so we don't always get the terrain grid as the collided grid!)
			//nonsense since i dont use scan ! terrainGrid->mTypeMask |= PhysicalZoneObjectType;
	
			// add grid to the scene
			terrainGrid->registerObject();
	
			// set the terrain grid index variable
			//this->mTerrainGridIndex = this->mGrids.size() - 1;
		}
		updateNodeCount();
	} //if 	(Con::getIntVariable("$Server::CreateMissionAreaGrid",true) {

	//XXTH Con::iAIMessagef("Immersive AI :: Seek :: PathMap Built!");
	Con::printf("Immersive AI :: Seek :: PathMap Built!");
	return true;
}

//------------------------------------------------------------------------------
void iAIPathMap::updateNodeCount() 
{
	// iterate over all grids to calculate total node count
	for (U32 i = 0; i < this->mGrids.size(); ++i)
		iAIPathMap::smNodeCount += this->mGrids[i]->mNodes.size();
}
//------------------------------------------------------------------------------



void iAIPathMap::clearMap()
{
	// iterate over nodes and delete all
	for (U32 i = 0; i < this->mGrids.size(); ++i)
	{
		if ((this->mGrids[i]) && (!this->mGrids[i]->isDeleted()))
			this->mGrids[i]->deleteObject();
		this->mGrids[i] = 0;
	}

	// set as uncompiled
	this->mCompiled = false;
	//this->mTerrainGridIndex = 0;
	this->mTerrainGridObj = NULL;
	iAIPathMap::smNodeCount = 0;
}

void iAIPathMap::toggleDisplay()
{
	// iterate over all grids
	for (U32 i = 0; i < this->mGrids.size(); ++i)
	{
		// toggle the display
		this->mGrids[i]->toggleDisplay();
	}
}

//-----------------------------------------------------------------------------------------------
//XXTH lets rock ! 
iAIPathGrid * iAIPathMap::addTerrainGrid( Point3F lGridStart, Point3F lGridEnd , Vector<Box3F> lAvoidList) 
{

    
	bool result = false;

    iAIPathGrid *lNewGrid = new iAIPathGrid();
	// generate a grid for the terrain, avoiding the avoid list
	if (lNewGrid->createTerrainGrid(lGridStart, lGridEnd, mMapBitmapHandle,  lAvoidList, IAIPATHGLOBAL_GRID_DENSITY_TERRAIN))
	{
		// add to pathmap collection
		this->mGrids.push_back(lNewGrid);

		// add grid to the scene
		lNewGrid->registerObject();

		updateNodeCount();
		result = true;

	}

	if (result)
	{
		return lNewGrid;
	} else {
		SAFE_DELETE_OBJECT(lNewGrid);
		return NULL;
	}
}
//-----------------------------------------------------------------------------------------------
//XXTH lets rock ! 
iAIPathGrid * iAIPathMap::addCustomGrid( Point3F lGridStart, Point3F lGridEnd ,  F32 lDensitiy, U32 lTypeMask) 
{

    bool result = false;
    Vector<Box3F> lAvoidList = 0; //XXTH BSD NULL;


    iAIPathGrid *lNewGrid = new iAIPathGrid();
	// generate a grid for the terrain, avoiding the avoid list
	if (lNewGrid->createCustomGrid(lGridStart, lGridEnd,  lAvoidList,  lDensitiy, lTypeMask))
	{
		// add to pathmap collection
		this->mGrids.push_back(lNewGrid);

		// add grid to the scene
		lNewGrid->registerObject();

		updateNodeCount();
		result = true;
	}


	if (result)
	{
		return lNewGrid;
	} else {
		SAFE_DELETE_OBJECT(lNewGrid);
		return NULL;
	}
}


//-----------------------------------------------------------------------------------------------
//XXTH lets rock again ;) 2023-03-10! 
iAIPathGrid* iAIPathMap::initCustomGrid(S32 lGridSize)
{

   iAIPathGrid* lNewGrid = new iAIPathGrid();
   lNewGrid->initCustomGrid(lGridSize);
   // add grid to the scene
   lNewGrid->registerObject();
   return lNewGrid;
}
//XXTH lets rock again ;) 2023-03-10! 
bool iAIPathMap::compileCustomGrid(iAIPathGrid* lGrid)
{

   if (!lGrid)
      return false;

   if (lGrid->compileCustomGrid())
   {
      // add to pathmap collection
      this->mGrids.push_back(lGrid);

      updateNodeCount();
   }

   return true;
}

//-----------------------------------------------------------------------------------------------
//XXTH lets rock again! 
iAIPathGrid * iAIPathMap::addCustomGridfromMarkers( SimPath::Path *pathObject) 
{

    bool result = false;
	Vector<Box3F> lAvoidList = 0; //XXTH BSD NULL;


    iAIPathGrid *lNewGrid = new iAIPathGrid();
	// generate a grid for the terrain, avoiding the avoid list
	if (lNewGrid->createCustomGridFromMarkers(pathObject))
	{
		// add to pathmap collection
		this->mGrids.push_back(lNewGrid);


		updateNodeCount();
		result = true;
	}


	if (result)
	{
		return lNewGrid;
	} else {
		SAFE_DELETE_OBJECT(lNewGrid);
		return NULL;
	}
}

//-----------------------------------------------------------------------------------------------

bool iAIPathMap::validPos(Point3F lPos)
{
	//bool maybeOK = false;
	//Check for bounds
	// iterate over all grids
	for (U32 i = 0; i < this->mGrids.size(); ++i)
	{
		// toggle the display
		if (this->mGrids[i]->mGridBox.isContained(lPos)) {

			return true;
			break;
		}
		
	}
 
	return false;
/*
    if (!maybeOK) return false;
//XXTH remove me when Interior is ok !!! !! 
	//Check for is Inside
   RayInfo lCollision;
   if(gServerContainer.castRay(Point3F(lPos.x, lPos.y, lPos.z + 0.5f), Point3F(lPos.x, lPos.y, lPos.z - 2000.f), InteriorObjectType, &lCollision))
   {
	   if(lCollision.face == -1) {
         return false;
	  }  else {

         InteriorInstance *lInterior = static_cast<InteriorInstance*>(lCollision.object);
		 

		 if(lInterior) {
             return( !(lInterior->getDetailLevel(0)->isSurfaceOutsideVisible(lCollision.face)) );
		 } else {
			return false;
		 }

      }
   } //raycast



	return true;
*/
}
//-----------------------------------------------------------------------------------------------
S32 iAIPathMap::getGridIndex(Point3F position)
{

	//This may suck with overlapping gridboxes :(
	for (S32 i = this->mGrids.size()-1; i >=0; --i)
	{
		// toggle the display
		if (this->mGrids[i]->mGridBox.isContained(position)) {
			return i;
		}
		
	}
   

	return -1;

}

//-----------------------------------------------------------------------------------------------

iAIPathNode* iAIPathMap::getClosestNode(const Point3F position)
{

	S32 gi = this->getGridIndex(position);
	if (gi >= 0) 
		return this->mGrids[gi]->getClosestNode(position);
	return NULL;
}
//------------------------------------------------------------------------------------------------------------
U32 iAIPathMap::updateMoveModifierBox(Box3F Box, U8 newModifier)
{

    S32 gi = this->getGridIndex(Box.minExtents);
	if (gi < 0) 
		return 0;
	return this->mGrids[gi]->updateMoveModifierBox(Box, newModifier);
}
//------------------------------------------------------------------------------------------------------------
U32 iAIPathMap::restoreMoveModifierBox(Box3F Box)
{
    S32 gi = this->getGridIndex(Box.minExtents);
	if (gi < 0) 
		return 0;
	return this->mGrids[gi]->restoreMoveModifierBox(Box);
}

//------------------------------------------------------------------------------------------------------------
void iAIPathMap::setMapBitmap(const char *name)
{

   if (*name) 
   {
      //mMapBitmapHandle = GFXTexHandle(name, BitmapKeepTexture);
      mMapBitmapHandle = GFXTexHandle(name, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
      


   }
   else 
   {
      // Reset handles if UI object is hidden
	  mMapBitmapHandle = NULL;
   }
}
//------------------------------------------------------------------------------------------------------------



ConsoleMethodGroupBegin(iAIPathMap, ScriptFunctions, "iAIPathMap Script Functions");

//DefineEngineStringlyVariadicMethod( iAIPathMap, initialize, bool, 2, 2,
DefineEngineMethod(iAIPathMap, initialize, bool, (), ,
			  "bool iAIPathMap.initialize() - Initializes the PathMap for the current mission.")
{
	return object->initialize();
}

//DefineEngineStringlyVariadicMethod( iAIPathMap, toggleDisplay, void, 2, 2,
DefineEngineMethod(iAIPathMap, toggleDisplay, void, (), ,
      "void iAIPathMap.toggleDisplay() - Toggles displaying of the pathmap.")
{
	object->toggleDisplay();
}

//DefineEngineStringlyVariadicMethod( iAIPathMap, closestNode, const char *, 3, 3,
DefineEngineMethod(iAIPathMap, closestNode, Point3F, (Point3F position), ,
      "Point3F iAIPathMap.closestNode(Point3F pos) - Get the closest node to the supplied position.")
{

   // get the closest node
   iAIPathNode* closestNode = object->getClosestNode(position);
   if (closestNode)
      return closestNode->getPosition();
   else
      return Point3F(0.f, 0.f, 0.f);


/*
	// ensure pos passed
	if (dStrlen(argv[2]) != 0) {

		// pass the args into a Point3F
		Point3F position;
		dSscanf(argv[2], "%f %f %f", &position.x, &position.y, &position.z);

		// get the closest node
		iAIPathNode* closestNode = object->getClosestNode(position);

		// ensure found a closest index
		if (closestNode)
		{
			char *returnBuffer = Con::getReturnBuffer(256);
			dSprintf(returnBuffer, 256, "%f %f %f", closestNode->mPosition.x, closestNode->mPosition.y, closestNode->mPosition.z);

			return returnBuffer;
		} else
		{
			Con::errorf("Immersive AI :: Seek :: PathMap - no node found near %f %f %f", position.x, position.y, position.z);
			return "";
		}
	} else
	{
		Con::errorf("Immersive AI :: Seek :: PathMap - no Point3F parsed to ClosestNode!");
		return "";
	}
*/
}

//-----------------------------------------------------------------------------------------------------------------------------
// 
//DefineEngineStringlyVariadicMethod( iAIPathMap, setMapBitmap, void, 3, 3, "(string filename)"
DefineEngineMethod(iAIPathMap, setMapBitmap, void, (String fileName), ,
              "Set the bitmap to set the node weights. Note that it is limited in size, to 512x512.")
{
   object->setMapBitmap(fileName.c_str());
}
//-----------------------------------------------------------------------------------------------------------------------------

//DefineEngineStringlyVariadicMethod( iAIPathMap, updateMoveModifierBox, S32, 5, 5, "updateMoveModifierBox (Point2F min,Point2F max,U8 newModifier (0=best walkable, 255 unwalkable, -1 restore initial value)" )
DefineEngineMethod(iAIPathMap, updateMoveModifierBox, bool, (Point2F min, Point2F max, U8 newModifier ), ,
   "updateMoveModifierBox (Point2F min,Point2F max,U8 newModifier (0=best walkable, 255 unwalkable, -1 restore initial value)")
{
   Box3F box(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
   box.minExtents.set(min.x, min.y, 0.f);
   box.maxExtents.set(max.x, max.y, 0.f);

   return object->updateMoveModifierBox(box, newModifier);

   /*
   Box3F box(0,0,0,0,0,0);
    dSscanf(argv[2],"%g %g",&box.minExtents.x,&box.minExtents.y);
    dSscanf(argv[3],"%g %g",&box.maxExtents.x,&box.maxExtents.y);

	S32 newModifier = dAtoi( argv[4] );
	if (newModifier >255) {
		Con::errorf("updateMoveModifierBox :: New modifier to big = %d !! ", newModifier);
		return 0;
	}
	if (newModifier<0) {
		return object->restoreMoveModifierBox(box);
	}
	
    return object->updateMoveModifierBox(box, newModifier);
    */
}
//-----------------------------------------------------------------------------------------------------------------------------
//DefineEngineStringlyVariadicMethod( iAIPathMap, addTerrainGrid, S32, 4, 4,
DefineEngineMethod(iAIPathMap, addTerrainGrid, S32, (Point2F min, Point2F max), ,
   "addTerrainGrid (Point2F min,Point2F max" )
{
   Point3F gridStart(min.x, min.y, -10.f);
   Point3F gridEnd(max.x, max.y, 1000.f);

   iAIPathGrid* lGrid = object->addTerrainGrid(gridStart, gridEnd);

   if (lGrid)
      return lGrid->getId();
   else
      return 0;


   /*
   Point3F gridStart;
   Point3F gridEnd;

    dSscanf(argv[2],"%g %g",&gridStart.x,&gridStart.y);
    dSscanf(argv[3],"%g %g",&gridEnd.x,&gridEnd.y);
    gridStart.z=-10.f;
	gridEnd.z=1000.f;

    iAIPathGrid * lGrid= object->addTerrainGrid(gridStart,gridEnd);

	if (lGrid)
		return lGrid->getId();
	else
		return 0;
    */
}
//-----------------------------------------------------------------------------------------------------------------------------
//DefineEngineStringlyVariadicMethod( iAIPathMap, getTerrainGrid, S32, 2, 2,
DefineEngineMethod(iAIPathMap, getTerrainGrid, S32, (), ,
      "return ID of main Terrain Grid" )
{
  return object->getTerrainGridObj()->getId();
}
//-----------------------------------------------------------------------------------------------------------------------------
//DefineEngineStringlyVariadicMethod( iAIPathMap, addCustomGrid, S32, 5, 6,
DefineEngineMethod(iAIPathMap, addCustomGrid, S32, (Point3F gridStart, Point3F gridEnd, F32 density, U32 typeMask),(1.f, IAIPATHGLOBAL_CUSTIOM_COLLISION_MASK),
   "addCustomGrid (Point2F min,Point2F max, F32 density, U32 typeMask " )
{

   iAIPathGrid* lGrid;
   lGrid = object->addCustomGrid(gridStart, gridEnd, density, typeMask);

   if (lGrid)
      return lGrid->getId();
   else
      return 0;

/*
   Point3F gridStart(min.x, min.y, -10.f);
   Point3F gridEnd(max.x, max.y, 1000.f);

   Point3F gridStart;
	Point3F gridEnd;

    dSscanf(argv[2],"%g %g %g",&gridStart.x,&gridStart.y,&gridStart.z);
    dSscanf(argv[3],"%g %g %g",&gridEnd.x,&gridEnd.y,&gridEnd.z);
	F32 ldent = dAtof(argv[4]);
    

	iAIPathGrid * lGrid;
	if (argc > 5) 
		lGrid = object->addCustomGrid(gridStart,gridEnd, ldent, dAtoi(argv[5]));
	else
		lGrid = object->addCustomGrid(gridStart,gridEnd, ldent);

	if (lGrid)
		return lGrid->getId();
	else
		return 0;
*/
}
//-----------------------------------------------------------------------------------------------------------------------------
//DefineEngineStringlyVariadicMethod( iAIPathMap, addCustomGridfromMarkers, S32, 3, 3,
DefineEngineMethod(iAIPathMap, addCustomGridfromMarkers, S32, (SimPath::Path* PathObj), ,
   "addCustomGridfromMarkers (Path object) NOTE: msToNext is used to limit the linkdistance!!" )
{
   iAIPathGrid* lGrid;
   lGrid = object->addCustomGridfromMarkers(PathObj);
   if (lGrid)
      return lGrid->getId();

   return 0;

   /*
   SimPath::Path *Obj;
   if( Sim::findObject( argv[2], Obj ) ) 
   {
	  iAIPathGrid * lGrid;
	  lGrid = object->addCustomGridfromMarkers(Obj);
	  if (lGrid)
		  return lGrid->getId();
   }
   return 0;
   */
}

//-----------------------------------------------------------------------------------------------------------------------------
// 2023-03-10
//-----------------------------------------------------------------------------------------------------------------------------

DefineEngineMethod(iAIPathMap, initCustomGrid, S32, (S32 lGridSize),(32),
   "init a new custom grid where you can add and link nodes... dont forget compile customgrid!!"
)
{

   iAIPathGrid* lGrid;
   lGrid = object->initCustomGrid(lGridSize);
   if (lGrid)
      return lGrid->getId();

   return 0;
}

DefineEngineMethod(iAIPathMap, compileCustomGrid, bool, (iAIPathGrid* lGrid), ,
   "compile and finallize a customGrid ... ")
{
   return object->compileCustomGrid(lGrid);
}



ConsoleMethodGroupEnd(iAIPathMap, ScriptFunctions);
