//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
/*-----------------------------------------------------------------------------
 BasicGrid
 Sounds Basic but have some cool features! 
 The node can use flags for a lot of stuff and pathfinding is included !!
 It use the tBinaryHeap from the 3th party tool immersiveAI to put the nodes
 on a heap fast. For Pathfinding there is a node weight (0..255) 255 means not
 traversal but could look strange while the path could be diagonal and it goes
 over the corner of a not walkeable block. But this is ok i think.

 Usage Example:
  %this.screen.grid =  new BasicGrid(); //BasicGrid_Create(gamescreen.getWorldRect(), 128);
  %this.screen.grid.init(gamescreen.getWorldRect(), %this.tileSize);
  ---
  %this.screen.grid.setIntValue(%worldX,%worldY,0,$EDITOR::brush);
  %this.screen.grid.setWeight(%worldX,%worldY,0);
  ---
  %this.path = %this.screen.grid.findPath(%start,%dest);


  Very cool in 2D with the object: tom2DGroundObject usage like this:
	  %this.groundRender = new tom2DGroundObject()
	  {
			layer  = 60;
			Layers = 1;
	  };
	  %this.groundRender.setTexture(%this.ground_tiles);
	=>  %this.groundRender.setGrid(%this.screen.grid);
	  %this.screen.addRenderObject(%this.groundRender);



 [ ] TODO: Save/Load a grid with its flags!


 2020-03-16 port to t3d
  TEST: $grid =  new BasicGrid(); $grid.init("0 0 100 100", 2);echo($grid.getNodesByRect("10 10 20 20"));


  


-----------------------------------------------------------------------------*/


#include "basicgrid.h"
#include "core/util/safeDelete.h"
#include "ohmtal/ai/aiMath.h"
#include "ohmtal/misc/tBinaryHeap.h"



/* Constructor */
BasicGrid::BasicGrid()
{
  
  setSquareSize(8); //2023-04-11 0 is bad ?! - think it is 
  mInitDone	  = false;
  mNodes      = NULL;
  mDebugGrid = false; 
  mClientGame = false;
  mDirty    = false;

  mNodesX = 0;
  mNodesY = 0;
  mNodeCount = 0;


}

//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(BasicGrid);

/* Destructor */
BasicGrid::~BasicGrid()
{
	mInitDone	  = false;
	SAFE_DELETE_ARRAY(mNodes);
}
//-----------------------------------------------------------------------------------------------------
/* 
TODO:
bool BasicGrid::save(const char *filename)
{
   FileStream writeFile;
   if (!ResourceManager->openFileForWrite(writeFile, filename))
      return false;

   // write the VERSION and HeightField
   writeFile.write((U8)FILE_VERSION);

   return (writeFile.getStatus() == FileStream::Ok);
}
*/
//-----------------------------------------------------------------------------------------------------
bool BasicGrid::onAdd()
{
   if (!Parent::onAdd())
      return false;


//   linkNamespaces();
   // Call onAdd in script!
   //Con::executef(this, 2, "onAdd", Con::getIntArg(getId()));
   Con::executef(this, "onAdd", getId()); // , getIdString());
   return true;
}
//------------------------------------------------------------------------------
void BasicGrid::onRemove()
{
   // Call onRemove in script!
   //Con::executef(this, 2, "onRemove", Con::getIntArg(getId()));
  Con::executef(this, "onRemove", getId());

//   unlinkNamespaces();

   Parent::onRemove();
}
//-----------------------------------------------------------------------------
void BasicGrid::init(RectI lArea, F32 lSquareSize)
{
	mInitDone = false;
	SAFE_DELETE_ARRAY(mNodes);
	mArea    = lArea; 
	
	setSquareSize( lSquareSize );

	mNodesX = mCeil(mArea.len_x() / getSquareSize());
	mNodesY = mCeil(mArea.len_y() / getSquareSize());
	mNodeCount = mNodesX * mNodesY;

	mNodes = new BasicGridNode[getNodeCount()];

	S32 i,j,n;
	BasicGridNode *lNode;
	n = 0;

	for (i=0; i<mNodesX; i++)
		for (j=0; j<mNodesY; j++)
		{

			lNode = &mNodes[i + j * mNodesX];
         lNode->mGrid = this;
			//using top left position!
			Point3F lPos = Point3F(mArea.point.x + i * getSquareSize(), mArea.point.y + j * getSquareSize(), 0.f);

			lNode->setPos(lPos);
			n++;

		}

   //only for node flags !! mDirty = true;
   mInitDone = true;
}

void BasicGrid::PrintInfo(bool listNodes)
{
   Con::printf("BASIC Grid - id:%d, Area: %d,%d %d,%d NodeCount:%d SquareSize:%f",
      getId(),
      getArea().point.x, getArea().point.y,
      getArea().extent.x, getArea().extent.y,
      getNodeCount(),
      getSquareSize());


   if (listNodes)
   {
      BasicGridNode* curNode;
      for (U32 i = 0; i < getNodeCount(); ++i)
      {
         curNode = getNodeById(i);
         Con::printf("node %d at %f,%f,%f (center: %f,%f,%f)", i,
            curNode->getPos().x, curNode->getPos().y, curNode->getPos().z,
            curNode->getCenterPos().x, curNode->getCenterPos().y, curNode->getCenterPos().z
         );
      }
   }


}

//-----------------------------------------------------------------------------------------------------
S32 BasicGrid::getNodeIndex(F32 x, F32 y, bool lPrec)
{
	S32 xNode;
	S32 yNode;

   //position out of area 
   if (!mArea.pointInRect(Point2I((U32)x, (U32)y)))
      return -1;


	if (lPrec)
	{
      xNode = mFloor(((x - mArea.point.x) / getSquareSize()));
      yNode = mFloor(((y - mArea.point.y) / getSquareSize()));

	} else {
		xNode = ((x - mArea.point.x) / getSquareSize());
		yNode = ((y - mArea.point.y) / getSquareSize());
	}
	

   if (xNode < 0 || xNode > mNodesX || yNode < 0 || yNode > mNodesY)
      return -1;
   
   S32 node = xNode + yNode * mNodesX;
   if(node >= 0 && node < getNodeCount())
      return node;

   return -1;
}
//-----------------------------------------------------------------------------------------------------
BasicGridNode* BasicGrid::findNode(F32 x, F32 y)
{
	S32 lNodeIndex = getNodeIndex(x,y, true);
	if (lNodeIndex >= 0)
		return &mNodes[lNodeIndex];
	else
		return NULL;

}

BasicGridNode* BasicGrid::findNode(F32 x, F32 y, S32& nodeIndex)
{
   nodeIndex = getNodeIndex(x, y, true);
   if (nodeIndex >= 0)
      return &mNodes[nodeIndex];
   else
      return NULL;
}


//-----------------------------------------------------------------------------------------------------
bool BasicGrid::getNodesByRect(const RectF &lRect, Vector<S32> &lList, bool lCanOverlap)
{
	if (!lRect.isValidRect())
		return false;

	S32 xB,yB,xE,yE;
   // upper left
	xB = ((lRect.point.x - mArea.point.x) / getSquareSize());
	yB = ((lRect.point.y - mArea.point.y) / getSquareSize());
   // bottom right
	xE = ((lRect.point.x + lRect.extent.x - mArea.point.x) / getSquareSize());
	yE = ((lRect.point.y + lRect.extent.y - mArea.point.y) / getSquareSize());

	if (lCanOverlap)
	{
		if (xB < 0)
			xB = 0;
		if (yB < 0)
			yB = 0;
		if (xE > mNodesX)
			xE = mNodesX;
			
		if  (yE > mNodesY)
			yE = mNodesY;
		

	} else {
		if (xB < 0 || yB < 0 || xE > mNodesX || yE > mNodesY)
			return false;
	}

    for (S32 i=xB; i <=xE; i++)
		for (S32 j=yB; j <=yE; j++)
		{
			S32 lNode = i + j * mNodesX;
			if (lNode < getNodeCount())
				lList.push_back(lNode);
		}

   return true;
}
//-----------------------------------------------------------------------------------------------------
 /*
 Directions:
     1  2  3
	 4  X  5
	 6  7  8
 */
BasicGridNode * BasicGrid::getNeighbour(BasicGridNode* startNode,U8 direction, S32& nodeIndex)
{

	// BasicGridNode* BasicGrid::findNode(F32 x, F32 y)

	F32 x = startNode->getPos().x;
	F32 y = startNode->getPos().y;
	//F32 dif = getSquareSize();
	switch (direction)
	{
	    case 1:
			x -= getSquareSize();
			y -= getSquareSize();
 			break;
	    case 2:
			y -= getSquareSize();
			break;
	    case 3:
			x += getSquareSize();
			y -= getSquareSize();
			break;

	    case 4:
			x -= getSquareSize();
			break;

	    case 5:
			x += getSquareSize();
			break;

		case 6:
			x -= getSquareSize();
			y += getSquareSize();
			break;

		case 7:
			y += getSquareSize();
			break;

	    case 8:
			x += getSquareSize();
			y += getSquareSize();
			break;
	}

	return findNode(x, y, nodeIndex);

}
//-----------------------------------------------------------------------------------------------------
static S32 BINARYHEAP_COMPARE pathNodeFitnessCompare( const void* a, const void* b )
{
	// compare the fitness of a & b
	F32 aCol = ((BasicGridNode *)(a))->mFitness;
	F32 bCol = ((BasicGridNode *)(b))->mFitness;
	F32 diff = aCol - bCol;
	S32 reply = diff < 0 ? -1 : (diff > 0 ? 1 : 0);
	return reply;
}

SimObject * BasicGrid::createPath(Point3F start, Point3F end, const bool smoothPath )
{
	bool result = false;
	Vector<BasicGridNode*> replyList;

	BasicGridNode* startNode = findNode(start.x, start.y);
	BasicGridNode* goalNode = findNode(end.x, end.y);
	if (startNode && goalNode)
		result = this->generatePath(startNode,goalNode,replyList,smoothPath );

	
	if (result) 
	{

		SimObject * pathObject = new SimObject();
      F32 halfSquareSize = getSquareSize() / 2;

		for (U32 i = 0; i < replyList.size(); i++)
		{
			//too much Con::printf("Node %d : %f,%f)",i,replyList[i]->getPos().x,replyList[i]->getPos().y);

			char nbuf[64];dMemset(nbuf,0,64);
			dSprintf(nbuf,20,"node%d",i);
			const char *fieldName = StringTable->insert(nbuf);


         //XXTH 2021-04-24 / OGE3D 2024-01-04 changed to center and Point2I
         dSprintf(nbuf, 64, "%d %d",
            (U32)(replyList[i]->getPos().x + halfSquareSize),
            (U32)(replyList[i]->getPos().y + halfSquareSize)
         );
/*
			dSprintf(nbuf,64,"%f %f %f",
            replyList[i]->getPos().x,
            replyList[i]->getPos().y,
            replyList[i]->getPos().z);
*/
			pathObject->setDataField( fieldName, NULL, nbuf );

		}
		return pathObject;
	}
	return NULL;
}


bool BasicGrid::generatePath(BasicGridNode* startNode, BasicGridNode* goalNode, Vector<BasicGridNode*> &replyList, const bool smoothPath)
{

	//ignore unwalkable ot same node
	if (goalNode->getPathWeight() == BASICGRID_UNWALKABLE || startNode == goalNode)
		return false;

	// openList is a binary heap
	BinaryHeap<BasicGridNode*> openList(getNodeCount(), pathNodeFitnessCompare);

	// list of all nodes which were affected during this pathfinding
	Vector<BasicGridNode*> affectedList;
	affectedList.clear();


	// add start node to open list
	startNode->mHeuristicCostToGoal = estimateCostToGoal(startNode, goalNode);
	startNode->mFitness = startNode->mLowestCostFromStart + startNode->mHeuristicCostToGoal;
	startNode->mOpen = true;
	openList.push(startNode);



	// keep searching while nodes in open list
	while (openList.size() > 0 && openList.size() < 250 ) //max 250 open! XXTH 20180210
	{
		// first element is the lowest cost
		BasicGridNode* currentNode = openList.front();

		// remove the first element from the openList
		openList.pop();

		// add the current node to affected list
		affectedList.push_back(currentNode);

		// set it as closed
		currentNode->mOpen = false;
		currentNode->mClosed = true;

		// iterate over all its neighbours

  		for (U32 i = 1; i < 9; ++i)
		{
         S32 lNodeIndex;
			BasicGridNode* currentNeighbour = getNeighbour(currentNode,i, lNodeIndex);

			if (!currentNeighbour)
				continue;
			// add current neighbour to affected list
			affectedList.push_back(currentNeighbour);

			// its closed, or not walkable, ignore the neighbour
			if ( (currentNeighbour->mClosed)  || (currentNeighbour->getPathWeight() == BASICGRID_UNWALKABLE) )
				continue;

			// if its not open, add it
			if (!currentNeighbour->mOpen)
			{
				// set this neighbours parent as the current node
				currentNeighbour->mParent = currentNode;

				// set the lowest cost, heuristic and fitness
				currentNeighbour->mLowestCostFromStart = currentNeighbour->mParent->mLowestCostFromStart + estimateCostToGoal(currentNeighbour, currentNeighbour->mParent);
				currentNeighbour->mHeuristicCostToGoal = estimateCostToGoal(currentNeighbour, goalNode);
				currentNeighbour->mFitness = currentNeighbour->mLowestCostFromStart + currentNeighbour->mHeuristicCostToGoal + currentNeighbour->getPathWeight();

				// set as open node
				currentNeighbour->mOpen = true;

				// add to open list
				openList.push(currentNeighbour);
			} else
			{
				// see neighbour already has the current node as its parent
				if (currentNeighbour->mParent != currentNode)
				{
					// see if this neighbour is a quicker path
					F32 costFromThisNode = currentNode->mLowestCostFromStart 
						                   + estimateCostToGoal(currentNeighbour, currentNode) + currentNeighbour->getPathWeight();
					if (currentNeighbour->mLowestCostFromStart > costFromThisNode)
					{
						// neighbour is better
						currentNeighbour->mLowestCostFromStart = costFromThisNode;
						currentNeighbour->mParent = currentNode;
					}
				}

				// close the node
				currentNeighbour->mClosed = false;
			}

			// see if we have reached the end yet
			if (currentNeighbour == goalNode)
			{
				// go back over all the nodes parents and construct the path
				BasicGridNode* currentTraceNode = goalNode;

				// keep going whilst able to find a parent
				while (currentTraceNode) {

					// add the node to the reply list
					replyList.push_front(currentTraceNode);

					// set parent as parents parent
					currentTraceNode = currentTraceNode->mParent;
				}


				// smooth the path
				if (smoothPath)
				{
//FIXME					this->smoothPath(replyList);
				}

				// reset the affected node pathfinding variables
				this->resetNodeVariables(affectedList);
				affectedList.clear();

				// found a path - return happy
				return true;
			}
		}
	}

	// reset the affected node pathfinding variables
	this->resetNodeVariables(affectedList);
	affectedList.clear();

	// couldn't find a path!
	return false;
}

void BasicGrid::resetNodeVariables(Vector<BasicGridNode*> &affectedList)
{
	// iterate over the affected list and reset the path finding variables
	for (U32 i = 0; i < affectedList.size(); i++)
	{
		affectedList[i]->mFitness = 0.0f;
		affectedList[i]->mLowestCostFromStart = 0.0f;
		affectedList[i]->mHeuristicCostToGoal = 0.0f;
		affectedList[i]->mParent = 0;
		affectedList[i]->mOpen = false;
		affectedList[i]->mClosed = false;
	}
}

F32 BasicGrid::estimateCostToGoal(BasicGridNode* from, BasicGridNode* goal)
{
	return (goal->getPos() - from->getPos()).len();
}
//=============================================================================================

// ConsoleMethod(BasicGrid,init, void, 4, 4, "param: area: x y w h, F32 SquareSize")
//DefineEngineMethod(BasicGrid, init, void, (const char* areastr, F32 squareSize), ,"param: area: x y w h, F32 SquareSize")
DefineEngineMethod(BasicGrid, init, void, (RectI area, F32 squareSize), , "param: area: x y w h, F32 SquareSize")
{
   /*
  F32 lSquareSize;
  RectI lArea;

  dSscanf(areastr, "%d %d %d %d", &lArea.point.x, &lArea.point.y, &lArea.extent.x, &lArea.extent.y);
  lSquareSize = squareSize;

  object->init(lArea,lSquareSize);
  */
   object->init(area, squareSize);

}


// ConsoleMethod(BasicGrid,getNodeCount, S32, 2, 2, "get count of nodes")
DefineEngineMethod(BasicGrid, getNodeCount, S32, (), , "get count of nodes")
{
	
	return object->getNodeCount();
}



//ConsoleMethod(BasicGrid, getPos, const char *, 4, 4, "x,y; return pos ")
DefineEngineMethod(BasicGrid, getPos, Point3I, (F32 x, F32 y), , "x,y; return top left pos of a node by the world values ")
{
  BasicGridNode *lNode = object->findNode(x,y);
  Point3I lResult = Point3I(0, 0, 0);

  if (lNode)
  {
     lResult.x = (U32)lNode->getPos().x;
     lResult.y = (U32)lNode->getPos().y;
     lResult.z = (U32)lNode->getPos().z;
  }

  return lResult;
}

// 2023-12-14 getCenterPos
DefineEngineMethod(BasicGrid, getCenterPos, Point3F, (F32 x, F32 y), , "x,y; return Point3F centerPos of a node by the world values (see also getNodeCenter) ")
{

   BasicGridNode* lNode = object->findNode(x, y);
/* Point3I ?! 
   Point3I lResult = Point3I(0, 0, 0);


   if (lNode)
   {
      Point3F lCenterPos = lNode->getCenterPos();
      lResult.x = (U32)lCenterPos.x;
      lResult.y = (U32)lCenterPos.y;
      lResult.z = (U32)lCenterPos.z;
   }
*/
   Point3F  lResult(0.f, 0.f, 0.f);

   if (lNode)
      lResult = lNode->getCenterPos();

   return lResult;
}





// ConsoleMethod(BasicGrid, getFlags, const char *, 4, 4, "x,y; return flags ")
DefineEngineMethod(BasicGrid, getFlags, S32, (F32 x, F32 y), , "x,y; return flags ")

{
  
   BasicGridNode* lNode = object->findNode(x, y);
   U32 lResult = 0;
	  
  if (lNode)
  {
     lResult = (U32)lNode->getFlags();
  }

  return lResult;
}



//ConsoleMethod(BasicGrid, getNodeByPos, const char *, 4,4, "x,y; return nodeidx x y z flags ")
DefineEngineMethod(BasicGrid, getNodeByPos, const char*, (F32 x, F32 y), , "x,y; return nodeidx x y z flags ")
{
  char* rbuf = Con::getReturnBuffer(256);
  BasicGridNode *lNode;
  S32 lNodeIndex = object->getNodeIndex(x,y , true);
  if (lNodeIndex >= 0)
		lNode = object->getNodeById(lNodeIndex);
  else
		return "";

  // getNodeIndex
  if (lNode)
  {
	  dSprintf(rbuf, 256, "%d %f %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, lNode->getPos().z, (U32)lNode->getFlags());
	return rbuf;
  }

  return "";
}

//ConsoleMethod(BasicGrid, getNodeIdByPos, S32, 4, 4, "x,y; return S32 nodeidx ")
DefineEngineMethod(BasicGrid, getNodeIdByPos, S32, (F32 x, F32 y), ,  "x,y; return S32 nodeidx ")
{
   BasicGridNode* lNode;
   S32 lNodeIndex = object->getNodeIndex(x,y, true);
   if (lNodeIndex >= 0)
      lNode = object->getNodeById(lNodeIndex);
   else
      return -1;

   // getNodeIndex
   if (lNode)
   {
      return  lNodeIndex;
   }

   return -1;
}




//ConsoleMethod(BasicGrid, getNode, const char *, 3,3, "S32 NodeIndex,  return nodeidx x y z flags ")
DefineEngineMethod(BasicGrid, getNode, const char*, (S32 nodeIndex), , "S32 NodeIndex,  return nodeidx x y z flags ")
{
  char* rbuf = Con::getReturnBuffer(256);
  BasicGridNode *lNode;
  S32 lNodeIndex = nodeIndex;
  if (lNodeIndex >= 0)
		lNode = object->getNodeById(lNodeIndex);
  else
		return "";

  // getNodeIndex
  if (lNode)
  {
	  dSprintf(rbuf, 256, "%d %f %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, lNode->getPos().z, (U32)lNode->getFlags());
	return rbuf;
  }

  return "";
}

////FIXME getNodesByRect useful for doing nasty things in 2d like render background in visible range, should return a object like findpath!
//ConsoleMethod(BasicGrid, getNodesByRect , const char *, 3,3, "x y w h,  return nodeidx nodeidx .. ")
DefineEngineMethod(BasicGrid, getNodesByRect, const char*, (RectF area), , "x y w h,  return nodeidx nodeidx .. ")
{

/* mhhh argc not longer supported ... ?! 
   RectF area;
   if(argc == 3)
      dSscanf(argv[2], "%f %f %f %f", &area.point.x, &area.point.y, &area.extent.x, &area.extent.y);
   else if(argc == 6)
   {
      area.point.x = dAtof(argv[2]);
      area.point.y = dAtof(argv[3]);
      area.extent.x = dAtof(argv[4]);
      area.extent.y = dAtof(argv[5]);
   }
*/
  	Vector<S32> lVisRadiusList;
   
	object->getNodesByRect(area,lVisRadiusList,true);


	if (lVisRadiusList.size() == 0)
		return "";

	char* rbuf = Con::getReturnBuffer(1024);
	for (S32 i = 0; i < lVisRadiusList.size() ; i++)
	{
		if (i == 0)
			dSprintf(rbuf, 1024, "%d",lVisRadiusList[i]);
		else
			dSprintf(rbuf, 1024, "%s %d", rbuf,lVisRadiusList[i]);
	}

   return rbuf;

}

//ConsoleMethod(BasicGrid, getNeighbour, const char*, 4, 4, "S32 NodeIndex, S32 Direction,  return nodeidx x y z flags "
DefineEngineMethod(BasicGrid, getNeighbour, const char*, (S32 nodeIndex, S32 direction), , "S32 NodeIndex, S32 Direction,  return nodeidx x y z flags "
   "Directions:"
   "1  2  3"
   "4  X  5"
   "6  7  8"
)
{
   char* rbuf = Con::getReturnBuffer(256);
   BasicGridNode* lstartNode;
   BasicGridNode* lNode;
   S32 lNodeIndex = 0;
   S32 lStartNodeIndex = nodeIndex;
   S32 lDirection = direction;
   if (lNodeIndex >= 0 && (lDirection > 0 && lDirection < 9))
   {
      lstartNode = object->getNodeById(lStartNodeIndex);
      lNode = object->getNeighbour(lstartNode, lDirection, lNodeIndex);
   }
   else
      return "";

   // getNodeIndex
   if (lNode)
   {
      dSprintf(rbuf, 256, "%d %f %f %f %d", lNodeIndex, lNode->getPos().x, lNode->getPos().y, lNode->getPos().z, (U32)lNode->getFlags());
      return rbuf;
   }

   return "";
}




// ConsoleMethod(BasicGrid, setFlags,bool, 5, 5, "x,y; set flags ")
DefineEngineMethod(BasicGrid, setFlags, bool, (F32 x, F32 y, U32 flags), , "x,y; set flags ")
{
  BasicGridNode *lNode = object->findNode(x,y);
	  
  if (lNode)
  {
     //done at the node object->setDirty();
	  lNode->setFlags(flags);
	  return true;
  }

  return false;
}

//ConsoleMethod(BasicGrid, setIntValue,bool, 6, 6, "x,y, idx[0..9], Value; set flags ")
DefineEngineMethod(BasicGrid, setIntValue, bool, (F32 x, F32 y, S32 idx, S32 value), , "x,y, idx[0..9], Value; set flags ")
{

  if (idx > 9 || idx < 0)
	 return false;
  BasicGridNode *lNode = object->findNode(x,y);
	  
  if (lNode)
  {
     //done at the node ..not atm but when used it will ...object->setDirty();
	  lNode->setIntValue(idx,value);
	  return true;
  }

  return false;
}

//ConsoleMethod(BasicGrid, setWeight,bool, 5, 5, "x,y, U8 weight")
DefineEngineMethod(BasicGrid, setWeight, bool, (F32 x, F32 y, S32 weigth), , "x,y, U8 weight")
{
  BasicGridNode *lNode = object->findNode(x,y);
	  
  if (lNode)
  {
     //done at the node ..not atm but when used it will ...object->setDirty();
	  lNode->setWeight(weigth);
	  return true;
  }

  return false;
}

DefineEngineMethod(BasicGrid, getIntValue,S32, (F32 x,F32 y,S32 idx), , "x,y, idx[0..9]")
{
  if (idx > 9 || idx < 0)
	 return 0;
  BasicGridNode *lNode = object->findNode(x,y);
	  
  if (lNode)
  {
	  return lNode->getIntValue(idx);

  }

  return 0;
}


// ConsoleMethod(BasicGrid, setIntValueByNodeId, bool, 5, 5, "nodeId, idx[0..9], S32 Value; store an integer value on a grid block ")
DefineEngineMethod(BasicGrid, setIntValueByNodeId, bool, (S32 nodeId, S32 idx, S32 value), , "x,y, U8 value")
{
   if (idx > 9)
      return false;
   BasicGridNode* lNode = object->getNodeById(nodeId);

   if (lNode)
   {
      //done at the node ..not atm but when used it will ...object->setDirty();
      lNode->setIntValue(idx, value);
      return true;
   }

   return false;
}

//ConsoleMethod(BasicGrid, getIntValueByNodeId, S32, 4, 4, "nodeId, idx[0..9]")
DefineEngineMethod(BasicGrid, getIntValueByNodeId, S32, (S32 nodeId, S32 idx), , "nodeId, idx[0..9]")

{
   
   if (idx > 9)
      return 0;
   BasicGridNode* lNode = object->getNodeById(nodeId);

   if (lNode)
   {
      return lNode->getIntValue(idx);

   }

   return 0;
}



//ConsoleMethod(BasicGrid,getinfo,void,2,2,"Display Infos on Console")
DefineEngineMethod(BasicGrid, getinfo, void, (bool listNodes), (false), "Display Infos on Console")
{
   object->PrintInfo(listNodes);

}

//ConsoleMethod(BasicGrid,findPath,S32,4,5,"findPath (Point3F start, Point3F goal, bool smoothPath = true) - Create a path between the two points.")
DefineEngineMethod(BasicGrid, findPath, S32, (Point3F start, Point3F goal, bool smoothPath), (Point3F::Zero, Point3F::Zero, true), "findPath (Point3F start, Point3F goal, bool smoothPath = true) - Create a path between the two points.")
{

		SimObject * result = object->createPath(start,goal, smoothPath);
		if (result)
		{
			result->registerObject();
			return result->getId();
		}

		return 0;
}


//ConsoleMethod(BasicGrid, getWeightByNodeId, S32, 3, 3, "nodeId")
DefineEngineMethod(BasicGrid, getWeightByNodeId, S32, (S32 nodeId), , "")
{

   BasicGridNode* lNode = object->getNodeById(nodeId);

   if (lNode)
   {
      return lNode->getWeight();

   }

   return -1;
}

//ConsoleMethod(BasicGrid, setWeightByNodeId, bool, 4, 4, "nodeId, U8 weight")
DefineEngineMethod(BasicGrid, setWeightByNodeId, bool, (S32 nodeId, S32 weight), , "nodeId, U8 weight")
{

   BasicGridNode* lNode = object->getNodeById(nodeId);

   if (lNode)
   {
      //done at the node ..not atm but when used it will ...object->setDirty();
      lNode->setWeight(weight);
      return true;
   }

   return false;
}


DefineEngineMethod(BasicGrid, getNodeCenter, Point2I, (F32 x, F32 y), , "x,y; return centerPos Point2I of a node by the world values (see also getCenterPos)")
{
   BasicGridNode* lNode = object->findNode(x, y);

   Point2I lResult = Point2I(0, 0);

   if (lNode)
   {
      Point3F lCenterPos = lNode->getCenterPos();
      lResult.x = (U32)lCenterPos.x;
      lResult.y = (U32)lCenterPos.y;
   }

   return lResult;
}

//ConsoleMethod(BasicGrid, getNodeCenterbyId, const char*, 3, 3, "return centered Point2I pos ")
DefineEngineMethod(BasicGrid, getNodeCenterbyId, Point2I, (S32 nodeId), , "return centered Point2I pos  ")
{
   BasicGridNode* lNode = object->getNodeById(nodeId);
   if (lNode)
   {
      return lNode->getPos2I();
   }
   return Point2I(0,0);
}

//ConsoleMethod(BasicGrid, getNodeRectbyId, const char*, 3, 3, "return centered rectI pos / extent ")
DefineEngineMethod(BasicGrid, getNodeRectbyId, RectI, (S32 nodeId), , "return centered rectI pos / extent ")
{
   BasicGridNode* lNode = object->getNodeById(nodeId);
   if (lNode)
   {
      return RectI(
         (U32)(lNode->getPos().x),
         (U32)(lNode->getPos().y),
         (U32)(object->getSquareSize()),
         (U32)(object->getSquareSize())
      );
   }

   return RectI(0,0,0,0);
}



//--------------------------------------------------------------------------------------------------------------------------------
// BasicGridNode
//--------------------------------------------------------------------------------------------------------------------------------
void BasicGridNode::setPos(Point3F lPos)
{
   mPos = lPos;
   if (mGrid)
      mCenterPos = mPos + Point3F(mGrid->getSquareSize() / 2, mGrid->getSquareSize() / 2, 0.f);

}

void BasicGridNode::setDirty()
{
   mDirty = true;
   if (mGrid)
       mGrid->setDirty(); 
}
