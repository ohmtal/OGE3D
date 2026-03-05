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

#include "T3D/gameBase/gameConnection.h"
#include "math/mSplinePatch.h"
#include "platform/profiler.h"

#include "ohmtal/misc/tBinaryHeap.h"
#include "iAIPathFind.h"
#include "iAIPathMap.h"

iAIPathFind* iAIPathFind::mInstance = 0;

iAIPathFind* iAIPathFind::getInstance()
{
	// if an instance doesn't exist yet, create one!
	if (!mInstance)
	{
		mInstance = new iAIPathFind();
	}
	return mInstance;
}

static S32 BINARYHEAP_COMPARE pathNodeFitnessCompare( const void* a, const void* b )
{
	// compare the fitness of a & b
	F32 aCol = ((iAIPathNode *)(a))->mFitness;
	F32 bCol = ((iAIPathNode *)(b))->mFitness;
	F32 diff = aCol - bCol;
	S32 reply = diff < 0 ? -1 : (diff > 0 ? 1 : 0);
	return reply;
}

bool iAIPathFind::generatePath(iAIPathNode* startNode, iAIPathNode* goalNode, Vector<iAIPathNode*> &replyList, const bool smoothPath)
{
	PROFILE_START(iAIPathFind_generatePath);

	// openList is a binary heap
	BinaryHeap<iAIPathNode*> openList(iAIPathMap::smNodeCount, pathNodeFitnessCompare);

	// list of all nodes which were affected during this pathfinding
	Vector<iAIPathNode*> affectedList;


	// add start node to open list
	startNode->mHeuristicCostToGoal = estimateCostToGoal(startNode, goalNode);
	startNode->mFitness = startNode->mLowestCostFromStart + startNode->mHeuristicCostToGoal;
	startNode->mOpen = true;
	openList.push(startNode);

	// keep searching while nodes in open list
	while (openList.size() > 0 && openList.size() < 250 ) //max 250 open! XXTH 20180210
	{
		// first element is the lowest cost
		iAIPathNode* currentNode = openList.front();

		// remove the first element from the openList
		openList.pop();

		// add the current node to affected list
		affectedList.push_back(currentNode);

		// set it as closed
		currentNode->mOpen = false;
		currentNode->mClosed = true;

		// iterate over all its neighbours
		for (U32 i = 0; i < currentNode->mNeighbours.size(); ++i)
		{
			iAIPathNode* currentNeighbour = currentNode->mNeighbours[i];

			// add current neighbour to affected list
			affectedList.push_back(currentNeighbour);

			// its its closed, or not walkable, ignore the neighbour
			if ((currentNeighbour->mClosed) || (currentNeighbour->mMoveModifier >= IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL))
				continue;

			// if its not open, add it
			if (!currentNeighbour->mOpen)
			{
				// set this neighbours parent as the current node
				currentNeighbour->mParent = currentNode;

				// set the lowest cost, heuristic and fitness
				currentNeighbour->mLowestCostFromStart = currentNeighbour->mParent->mLowestCostFromStart + estimateCostToGoal(currentNeighbour, currentNeighbour->mParent);
				currentNeighbour->mHeuristicCostToGoal = estimateCostToGoal(currentNeighbour, goalNode);
				currentNeighbour->mFitness = currentNeighbour->mLowestCostFromStart + currentNeighbour->mHeuristicCostToGoal + currentNeighbour->mMoveModifier;

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
					F32 costFromThisNode = currentNode->mLowestCostFromStart + estimateCostToGoal(currentNeighbour, currentNode) + currentNeighbour->mMoveModifier;
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
				iAIPathNode* currentTraceNode = goalNode;

				// keep going whilst able to find a parent
				while (currentTraceNode) {

					// add the node to the reply list
					replyList.push_front(currentTraceNode);

					// set parent as parents parent
					currentTraceNode = currentTraceNode->mParent;
				}

				//XXTH Con::iAIMessagef("Immersive AI :: Seek :: Path found!");

				// Con::printf("Immersive AI :: Seek :: Path found!");
				

				// smooth the path
				if (smoothPath)
				{
					//Con::printf("Immersive AI :: Seek :: Smoothing path... %d nodes to start", replyList.size());
					this->smoothPath(replyList);
					//Con::printf("Immersive AI :: Seek :: Path smoothed... %d nodes now", replyList.size());
				}

				// reset the affected node pathfinding variables
				this->resetNodeVariables(affectedList);
				affectedList.clear();

				// found a path - return happy
				PROFILE_END(); //XXTH return here but missed PROFILE_END!
				return true;
			}
		}
	}

	// reset the affected node pathfinding variables
	this->resetNodeVariables(affectedList);
	affectedList.clear();

	// couldn't find a path!
	PROFILE_END();
	return false;
}

inline F32 iAIPathFind::estimateCostToGoal(iAIPathNode* from, iAIPathNode* goal)
{
	return (goal->mPosition - from->mPosition).len();
}

void iAIPathFind::smoothPath(Vector<iAIPathNode*> &replyList)
{
	PROFILE_START(iAIPathFind_smoothPath);
	U32 iter = 0;
	bool erasedNode = false;
	while (iter < (replyList.size()-2))
	{
		erasedNode = false;

		// check height difference from a [iter] to b [iter+1]
		Point3F vec = replyList[iter]->mPosition - replyList[iter+1]->mPosition;
		F32 zSq = vec.z * vec.z;

		// only see if we can remove b if less than the max slope
		// prevents things like making a path from one mountain top to another
		if (zSq < IAIPATHGLOBAL_MAX_SMOOTHED_SLOPE)
		{
			//XXTH really ?! ==> YES I GUESS 
			
			// determine angle difference
			F32 angle = mRadToDeg(mDot(replyList[iter]->mPosition, replyList[iter+2]->mPosition) / (replyList[iter]->mPosition * replyList[iter+2]->mPosition).len());

			// check if angle is within acceptable range
            if (((90-IAIPATHGLOBAL_PATH_SMOOTH_ANGLE_THRESHOLD) < angle) && (angle < (90+IAIPATHGLOBAL_PATH_SMOOTH_ANGLE_THRESHOLD)))
			
			if (true)
			{
				// check that it is a valid connection (avoiding terrain aswell)
				if (smoothPathConnectionValid(replyList[iter]->mPosition, replyList[iter+2]->mPosition))
				{
					// remove b [iter+1]
 					replyList.erase(iter+1);
					erasedNode = true;
				}
			}
		}

		// only increment the iter if no nodes were deleted from the path
		if (!erasedNode)
			iter++;
	}
	PROFILE_END();
}

bool iAIPathFind::smoothPathConnectionValid(Point3F from, Point3F to, U32 lmask)
{
	// adjust positions to check slightly above terrain
	from.z += 0.5f; //IAIPATHGLOBAL_NODE_CLEARANCE.z;
	to.z   += 0.5f; //IAIPATHGLOBAL_NODE_CLEARANCE.z;

	RayInfo dummy;

	
//Con::executef(4, "GlobalOnDebugRender", Con::getFloatArg(from.x),Con::getFloatArg(from.y),Con::getFloatArg(from.z));
//Con::executef(4, "GlobalOnDebugRender", Con::getFloatArg(to.x),Con::getFloatArg(to.y),Con::getFloatArg(to.z));

	// if we can't get from node to neighbour without colliding, it is untraversal
	if (gServerContainer.castRay(from, to, lmask, &dummy))
		return false;

	return true;
}

void iAIPathFind::resetNodeVariables(Vector<iAIPathNode*> &affectedList)
{
	PROFILE_START(iAIPathFind_resetNodeVariables);

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
	PROFILE_END();
}
