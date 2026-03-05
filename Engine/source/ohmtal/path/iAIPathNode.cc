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

#include "scene/sceneManager.h"
#include "collision/earlyOutPolyList.h"
#include "iAIPathNode.h"
#include "iAIPathMap.h"
#include "iAIPathGlobal.h"
#include "scene/sceneObject.h"


#include "iAIPathGlobal.h"
#include "iAIPathNode.h"
#include "iAIPathMap.h"

iAIPathNode::iAIPathNode(const Point3F position, iAIPathGrid* pathGrid, const U16 idX, const U16 idY, const U8 weight )
{
	this->mPosition = position;
	this->mIdX = idX;
	this->mIdY = idY;
	this->mParentGrid = pathGrid;

	this->mFitness = 0.0f;
	this->mLowestCostFromStart = 0.0f;
	this->mHeuristicCostToGoal = 0.0f;
	this->mParent = 0;
	this->mOpen = false;
	this->mClosed = false;
	this->mBadNode = false;
	this->mMoveModifier = weight;
	this->mSaveMoveModifier = weight;

}

iAIPathNode::~iAIPathNode()
{
	while (!this->mNeighbours.empty())
	{
		this->mNeighbours.erase((U32)0);
	}
	this->mNeighbours.clear();
}
//------------------------------------------------------------------------------------------------------------------------------
bool iAIPathNode::isClear(bool clearanceCheck) 
{
	if (!clearanceCheck) 
			return true;

   Point3F  center = this->mPosition + Point3F(0, 0, IAIPATHGLOBAL_NODE_CLEARANCE.z / 2);
   Point3F  extent = IAIPATHGLOBAL_NODE_CLEARANCE;
   U32  mask = IAIPATHGLOBAL_COLLISION_MASK;
   

   Box3F    B(center - extent, center + extent, true);

   EarlyOutPolyList polyList;
   polyList.mPlaneList.clear();
   polyList.mNormal.set(0,0,0);
   polyList.mPlaneList.setSize(6);
   polyList.mPlaneList[0].set(B.minExtents, VectorF(-1,0,0));
   polyList.mPlaneList[1].set(B.maxExtents, VectorF(0,1,0));
   polyList.mPlaneList[2].set(B.maxExtents, VectorF(1,0,0));
   polyList.mPlaneList[3].set(B.minExtents, VectorF(0,-1,0));
   polyList.mPlaneList[4].set(B.minExtents, VectorF(0,0,-1));
   polyList.mPlaneList[5].set(B.maxExtents, VectorF(0,0,1));


   //bool SceneContainer::buildPolyList(PolyListContext context, const Box3F &box, U32 mask, AbstractPolyList *polyList)
   bool result = ! gServerContainer.buildPolyList(PLC_Collision, B, mask, &polyList);

   return result;
}
//--------------------------------------------------------------------------------------------------------------------------------
/*

bool iAIPathNode::isClear()
{
	Point3F start;
	Point3F end;
	RayInfo dummy;

	//XXTH 5 point ray!

	//middle check 
	start = this->mPosition;
	end = this->mPosition + Point3F(0, 0, IAIPATHGLOBAL_NODE_CLEARANCE.z);
	if (gServerContainer.castRay(start, end, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;

	//left check 
	end = this->mPosition + Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x * -1.0f , 0, IAIPATHGLOBAL_NODE_CLEARANCE.z);;
	start = this->mPosition + Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x * -1.0f, 0, 0);
	if (gServerContainer.castRay(start, end, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;

	//right check 
	end = this->mPosition + Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x  , 0, IAIPATHGLOBAL_NODE_CLEARANCE.z);;
	start = this->mPosition + Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x , 0, 0);
	if (gServerContainer.castRay(start, end, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;

	//top check 
	end = this->mPosition + Point3F(0, IAIPATHGLOBAL_NODE_CLEARANCE.y , IAIPATHGLOBAL_NODE_CLEARANCE.z);;
	start = this->mPosition + Point3F(0, IAIPATHGLOBAL_NODE_CLEARANCE.y, 0);
	if (gServerContainer.castRay(start, end, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;
     
	//bottom check 
	end = this->mPosition + Point3F(0, IAIPATHGLOBAL_NODE_CLEARANCE.y * -1.0f, IAIPATHGLOBAL_NODE_CLEARANCE.z);;
	start = this->mPosition + Point3F(0, IAIPATHGLOBAL_NODE_CLEARANCE.y * -1.0f, 0);
	if (gServerContainer.castRay(start, end, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;

	// hopefully be valid
	return true;
}
*/
bool iAIPathNode::isNeighbourValid(const Point3F neighbourPosition, bool clearanceCheck)
{
    //XXTH weight 0 is bad :P
	// NO we keep it so we can change the weight later ! if (this->mMoveModifier==IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL) return false;

	// calculate vector in z
	Point3F vec = this->mPosition - neighbourPosition;

/* XXTH Slopdisabled ==> 2023 reanabled BUT  need something better for slope!!!  */
	F32 zSq = vec.z * vec.z;
	// ensure difference in node height is valid
	if (zSq > IAIPATHGLOBAL_MAX_SLOPE)
		return false;

	RayInfo dummy;
	
	// quick check from node to neighboour position
	if (gServerContainer.castRay(this->mPosition, neighbourPosition, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;


	if (!clearanceCheck) 
		return true;

	// check 4 points around clearance box
	Point3F offset = -Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x/2, 0, 0);
	if (gServerContainer.castRay(this->mPosition + offset, neighbourPosition + offset, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;
	offset = Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x/2, 0, 0);
	if (gServerContainer.castRay(this->mPosition + offset, neighbourPosition + offset, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;
	offset = Point3F(0, 0, IAIPATHGLOBAL_NODE_CLEARANCE.z);
	if (gServerContainer.castRay(this->mPosition + offset, neighbourPosition + offset, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;
	offset = Point3F(IAIPATHGLOBAL_NODE_CLEARANCE.x/2, 0, IAIPATHGLOBAL_NODE_CLEARANCE.z);
	if (gServerContainer.castRay(this->mPosition + offset, neighbourPosition + offset, IAIPATHGLOBAL_COLLISION_MASK, &dummy))
		return false;

	// must be valid
	return true;
}


//XXTH 2023-03-10
bool iAIPathNode::addNeighbourNoCheck(iAIPathNode* neighbour)
{
   if (neighbour && !neighbour->mBadNode && !this->hasNeighbour(neighbour)) //XXTH neighbour should not be a bad Node and no double neighbours!!!!
   {
         this->mNeighbours.push_back(neighbour);
         return true;
   }

   return false;
}


bool iAIPathNode::addNeighbour(iAIPathNode* neighbour,bool clearanceCheck)
{
	if (neighbour && !neighbour->mBadNode && !this->hasNeighbour(neighbour)) //XXTH neighbour should not be a bad Node and no double neighbours!!!!
	{
		// check that neighbour is valid before adding
		if (isNeighbourValid(neighbour->mPosition, clearanceCheck))
		{
			this->mNeighbours.push_back(neighbour);
			return true;
		}
		//
	}
    
	return false;
}

bool iAIPathNode::hasNeighbour(iAIPathNode* neighbour)
{
	if (neighbour)
	{
		// iterate over all neighbours
		for (U32 i = 0; i < this->mNeighbours.size(); i++)
		{
			// if position match, then it has that neighbour!
			if (this->mNeighbours[i]->mPosition == neighbour->mPosition)
				return true;
		}
	}
	return false;
}

bool iAIPathNode::removeNeighbour(iAIPathNode* neighbour)
{
	if (neighbour)
	{
		// iterate over all neighbours
		for (U32 i = 0; i < this->mNeighbours.size(); i++)
		{
			// if position match, then it has that neighbour!
			if (this->mNeighbours[i]->mPosition == neighbour->mPosition)
			{
				// remove the neighbour
				this->mNeighbours.erase(i);
				return true;
			}
		}
	}
	return false;
}
/* XXTH unused
void iAIPathNode::updateMoveModifier()
{
	RayInfo dummy;
	// check if the node is in water
	if (gServerContainer.castRay(this->mPosition + Point3F(0,0,1000.0f), this->mPosition - Point3F(0, 0, IAIPATHGLOBAL_NODE_CLEARANCE.z / 2), WaterObjectType, &dummy))
		this->mMoveModifier = IAIPATHGLOBAL_MOVE_MODIFIER_WATER;
	else
		this->mMoveModifier = 0.0f;
}
*/
