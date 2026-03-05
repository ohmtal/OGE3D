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

//-------------------------------------------------------------------
/// @file iAIPathMap.h
//-------------------------------------------------------------------
/// @class iAIPathMap
/// @author Gavin Bunney
/// @version 1.0
/// @brief Map of all nodes within the mission.
/// 
/// Holds a collection of all grids (collections of nodes) for the
/// current server map. The map creates grids and maps links between
/// them allowing for A* pathfinding.
//-------------------------------------------------------------------
#ifndef _IAIPATHMAP_H_
#define _IAIPATHMAP_H_

#include "iAIPathMap.h"
#include "iAIPathGrid.h"
#include "iAIPathNode.h"
#include "iAIPathGlobal.h"
#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

class iAIPathMap : public SimObject
{
	typedef SimObject Parent;
	friend class iAIPath;

private:
	//XXTH handle for mapbitmap
   GFXTexHandle mMapBitmapHandle;

public:
	
	//-------------------------------------------------------------------
	/// @var DECLARE_CONOBJECT(iAIPathMap)
	/// @brief TorqueScript object.
	//-------------------------------------------------------------------
	DECLARE_CONOBJECT(iAIPathMap);

	//-------------------------------------------------------------------
	/// @fn iAIPathMap() 
	/// @brief Default constructor.
	//-------------------------------------------------------------------
	iAIPathMap();

	//-------------------------------------------------------------------
	/// @fn ~iAIPathMap() 
	/// @brief Deconstructor which clears all the grids.
	//-------------------------------------------------------------------
	~iAIPathMap();

	//-------------------------------------------------------------------
	/// @fn bool initialize()
	/// @brief Initializes the pathmap for the current server map.
	///
	/// @return initialization success.
	//-------------------------------------------------------------------
	bool initialize();

	//-------------------------------------------------------------------
	/// @fn bool createPathMap()
	/// @brief Creates the pathmap for the current server map.
	///
	/// @return creation success.
	//-------------------------------------------------------------------
	bool createPathMap();

	//XXTH new void:
	void updateNodeCount();
	//-------------------------------------------------------------------
	/// @fn void clearMap()
	/// @brief Clears the map.
	//-------------------------------------------------------------------
	void clearMap();

	//-------------------------------------------------------------------
	/// @fn void toggleDisplay()
	/// @brief Toggle displaying of the pathmap.
	//-------------------------------------------------------------------
	void toggleDisplay();

	//-------------------------------------------------------------------
	/// @fn iAIPathNode* getClosestNode(const Point3F position)
	/// @brief Retrieves the closest node to the parsed position
	///
	/// @param position world point of node to find
	/// @return pointer to closest node
	//-------------------------------------------------------------------
	iAIPathNode* getClosestNode(const Point3F position);

    //XXTH set a new moveModifier for grid points inside the Box
	U32 updateMoveModifierBox(Box3F Box, U8 newModifier);
	U32 restoreMoveModifierBox(Box3F Box);

	S32 getGridIndex(Point3F position);


	//-------------------------------------------------------------------
	/// @fn static U32 smNodeCount
	/// @brief Total count of nodes in the Path Map.
	//-------------------------------------------------------------------
	static U32 smNodeCount;


	//XXTH void setbitmap
	void setMapBitmap(const char *name);

	//XXTH Method to add addional terrain grid:
	iAIPathGrid * addTerrainGrid(Point3F lGridStart, Point3F lGridEnd, Vector<Box3F> lAvoidList = 0);
	iAIPathGrid * addCustomGrid( Point3F lGridStart, Point3F lGridEnd ,  F32 lDensitiy = 1.f, U32 lTypeMask = IAIPATHGLOBAL_CUSTIOM_COLLISION_MASK ); 
	iAIPathGrid* initCustomGrid(S32 lGridSize = 32);
	bool compileCustomGrid(iAIPathGrid* lGrid);
	iAIPathGrid * addCustomGridfromMarkers( SimPath::Path *pathObject);



	bool validPos(Point3F lPos);

protected:

	//-------------------------------------------------------------------
	/// @var Vector<iAIPathGrid*> mGrids
	/// @brief Vector of all grids in the world.
	//-------------------------------------------------------------------
	Vector<iAIPathGrid*> mGrids;

	//-------------------------------------------------------------------
	/// @var bool mCompiled
	/// @brief Flag for when a pathmap has been compiled successfully.
	//-------------------------------------------------------------------
	bool mCompiled;

	//-------------------------------------------------------------------
	/// @var U32 mTerrainGridIndex
	/// @brief Holds the index of the terrain grid within mGrids.
	//-------------------------------------------------------------------
	//U32 mTerrainGridIndex;

	
	iAIPathGrid * mTerrainGridObj;

public:
	iAIPathGrid * getTerrainGridObj() { return mTerrainGridObj; }
};

#endif
