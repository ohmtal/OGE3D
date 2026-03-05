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
/// @file iAIPathFind.h
//-------------------------------------------------------------------
/// @class iAIPathFind
/// @author Gavin Bunney
/// @version 1.0
/// @brief Finds a path from one node to another.
/// 
/// A Singleton class which implements an A* pathfinding algorithm
/// to find the easiest and shortest path from one node to another.
//-------------------------------------------------------------------
#ifndef _IAIPATHFIND_H_
#define _IAIPATHFIND_H_

#include "iAIPathNode.h"
#include "iAIPathGlobal.h"

class iAIPathFind {

public:

	//-------------------------------------------------------------------
	/// @fn ~iAIPathFind() 
	/// @brief Default deconstructor.
	//-------------------------------------------------------------------
	~iAIPathFind() { };

	//-------------------------------------------------------------------
	/// @fn static iAIPathFind* getInstance()
	/// @brief Retrieves the instance of the singleton. Creates a new
	///        instance if one doesn't exist.
	///
	/// @return Singleton instance pointer.
	//-------------------------------------------------------------------
	static iAIPathFind* getInstance();

	//-------------------------------------------------------------------
	/// @fn bool generatePath(iAIPathNode* startNode,
	///                       iAIPathNode* goalNode,
	///                       Vector<iAIPathNode*> &replyList,
	///                       const bool smoothPath = true)
	/// @brief Performs an A* path finding algorithm to find a path from 
	///        the parsed startNode to the goalNode. Path is returned in
	///        the replyList.
	///
	/// @param startNode Pointer to the start node.
	/// @param goalNode Pointer to the goal node.
	/// @param replyList Vector to place the returned path in.
	/// @param smoothPath Flag to smooth the path. Default true.
	/// @return Path creation success.
	//-------------------------------------------------------------------
	bool generatePath(iAIPathNode* startNode, iAIPathNode* goalNode, Vector<iAIPathNode*> &replyList, const bool smoothPath = true);


	//-------------------------------------------------------------------
	/// @fn bool smoothPathConnectionValid(const Point3F from,
	///          const Point3F to)
	/// @brief Checks that a connection from one point to another is
	///        valid.
	///
	/// @param from Point to start from.
	/// @param from Point to go to.
	/// @return True if connection is valid.
	//-------------------------------------------------------------------
	// XXTH moved to public!
	bool smoothPathConnectionValid(Point3F from, Point3F to, U32 lMask = ( StaticShapeObjectType |  TerrainObjectType | TerrainLikeObjectType | InteriorLikeObjectType ));

private:

	//-------------------------------------------------------------------
	/// @fn iAIPathFind::iAIPathFind() 
	/// @brief Default constructor.
	//-------------------------------------------------------------------
	iAIPathFind() { };

	//-------------------------------------------------------------------
	/// @var static iAIPathFind* mInstance
	/// @brief Instance of the singleton.
	//-------------------------------------------------------------------
	static iAIPathFind* mInstance;

	//-------------------------------------------------------------------
	/// @fn inline F32 estimateCostToGoal(iAIPathNode* node,
	///                                   iAIPathNode* goal)
	/// @brief Performs a vector length from node to goal, to determine
	///        the cost from the node to the goal.
	///
	/// @param node Pointer to the start node.
	/// @param goal Pointer to the goal node.
	/// @return Cost to goal.
	//-------------------------------------------------------------------
	inline F32 estimateCostToGoal(iAIPathNode* node, iAIPathNode* goal);

	//-------------------------------------------------------------------
	/// @fn void smoothPath(Vector<iAIPathNode*> &replyList)
	/// @brief Smooths the path to further optimise the A* algorithm and
	///        make AI agents traverse a path more naturally.
	///
	/// @param replyList Vector of iAIPathNode's to smooth.
	//-------------------------------------------------------------------
	void smoothPath(Vector<iAIPathNode*> &replyList);


	//-------------------------------------------------------------------
	/// @fn void resetNodeVariables(Vector<iAIPathNode*> &affectedList)
	/// @brief Resets the path finding variables of the nodes.
	///
	/// @param affectedList Vector of nodes which were changed during
	///        the pathfinding algorithm.
	//-------------------------------------------------------------------
	void resetNodeVariables(Vector<iAIPathNode*> &affectedList);
};

#endif
