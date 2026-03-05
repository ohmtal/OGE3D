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
/// @file iAIPathNode.h
//-------------------------------------------------------------------
/// @class iAIPathNode
/// @author Gavin Bunney
/// @version 1.0
/// @brief Represents a single path finding node.
/// 
/// A point within the game world, consisting of a location, id, 
/// neighbours and various path finding variables.
//-------------------------------------------------------------------


#ifndef _IAIPATHNODE_H_
#define _IAIPATHNODE_H_

class iAIPathGrid; //XXTH needed for linux build :P

class iAIPathNode {

	friend class iAIPathMap;
	friend class iAIPathGrid;
	friend class iAIPath;
	friend class iAIPathFind;

public:

	//-------------------------------------------------------------------
	/// @fn iAIPathNode() 
	/// @brief Default constructor.
	//-------------------------------------------------------------------
	iAIPathNode() { }

	//-------------------------------------------------------------------
	/// @fn ~iAIPathNode() 
	/// @brief Default deconstructor.
	//-------------------------------------------------------------------
	~iAIPathNode();

	//-------------------------------------------------------------------
	/// @fn iAIPathNode(const Point3F position, iAIPathGrid* pathGrid, 
	///                 const U16 idX, const U16 idY) 
	/// @brief Constructor to create node at specified position.
	///
	/// @param position Point in world coords to create the node.
	/// @param pathGrid Pointer to grid which the node is contained in.
	/// @param idX ID in X of the node within the grid.
	/// @param idY ID in Y of the node within the grid.
	//-------------------------------------------------------------------
	iAIPathNode(const Point3F position, iAIPathGrid* pathGrid, const U16 idX, const U16 idY, const U8 weight);

	//-------------------------------------------------------------------
	/// @fn bool isClear()
	/// @brief Checks if the node is in a valid position, clear of
	///	       obstructions.
	///
	/// @return Node clear of all obstructions.
	//-------------------------------------------------------------------
	bool isClear(bool clearanceCheck = true);

	//-------------------------------------------------------------------
	/// @fn bool addNeighbour(iAIPathNode* neighbour)
	/// @brief Adds a neighbour to this node's neighbour list.
	///
	/// @return Neighbour added.
	//-------------------------------------------------------------------
	bool addNeighbour(iAIPathNode* neighbour,bool clearanceCheck = true);

	//-------------------------------------------------------------------
	/// @fn bool hasNeighbour(iAIPathNode* neighbour)
	/// @brief Checks if the node has the parsed neighbour.
	///
	/// @return Neighbour in this node's neighbour list.
	//-------------------------------------------------------------------
	bool hasNeighbour(iAIPathNode* neighbour);

	//-------------------------------------------------------------------
	/// @fn bool removeNeighbour(iAIPathNode* neighbour)
	/// @brief Removes the specified neighbour.
	///
	/// @return Neighbour was removed successfully.
	//-------------------------------------------------------------------
	bool removeNeighbour(iAIPathNode* neighbour);

	//-------------------------------------------------------------------
	/// @fn void bool isNeighbourValid(const Point3F neighbourPosition)
	/// @brief Checks if a neighbour is accessible from this node.
	///
	/// @param neighbourPosition The position of the neighbour to check.
	/// @return Neighbour in valid position
	//-------------------------------------------------------------------
	bool isNeighbourValid(const Point3F neighbourPosition, bool clearanceCheck = true);

   bool addNeighbourNoCheck(iAIPathNode* neighbour);

	//-------------------------------------------------------------------
	/// @var Point3F mPosition
	/// @brief Position of the node in world coordinates.
	//-------------------------------------------------------------------
	Point3F mPosition;

	//-------------------------------------------------------------------
	/// @var U16 mIdX
	/// @brief ID in X within the node's grid.
	//-------------------------------------------------------------------
	U16 mIdX;

	//-------------------------------------------------------------------
	/// @var U16 mIdY
	/// @brief ID in Y within the node's grid.
	//-------------------------------------------------------------------
	U16 mIdY;

	//-------------------------------------------------------------------
	/// @var Vector<iAIPathNode*> mNeighbours
	/// @brief Vector of the nodes neighbours. Expect 8 neighbours, but
	///        if edge of area or obstructed, will be less.
	//-------------------------------------------------------------------
	Vector<iAIPathNode*> mNeighbours;

	//-------------------------------------------------------------------
	/// @var iAIPathGrid* mParentGrid
	/// @brief Pointer to the grid which the node is contained within.
	//-------------------------------------------------------------------
	iAIPathGrid* mParentGrid;

	//-------------------------------------------------------------------
	/// @fn void updateMoveModifier()
	/// @brief Updates the node's move modifer, based on its position.
	//-------------------------------------------------------------------
//XXTH unused	void updateMoveModifier();

	//-------------------------------------------------------------------
	/// @var U8 mMoveModifier
	/// @brief Utilised in A* algorithm; the level of difficulty at this
	//         node. 255 is untraversal, 0 is easiest.
    //  XXTH used from mapbitmap
	//-------------------------------------------------------------------
	U8 mMoveModifier;
	U8 mSaveMoveModifier;

	//-------------------------------------------------------------------
	/// @var F32 mFitness
	/// @brief Utilised in A* algorithm; fitness of this node from a
	///        start node.
	//-------------------------------------------------------------------
	F32 mFitness;

	//-------------------------------------------------------------------
	/// @var F32 mLowestCostFromStart
	/// @brief Utilised in A* algorithm; lowest cost from the start node.
	//-------------------------------------------------------------------
	F32 mLowestCostFromStart;

	//-------------------------------------------------------------------
	/// @var F32 mHeuristicCostToGoal
	/// @brief Utilised in A* algorithm; best guess cost to goal from this
	///        node.
	//-------------------------------------------------------------------
	F32 mHeuristicCostToGoal;

	//-------------------------------------------------------------------
	/// @var iAIPathNode* mParent
	/// @brief Utilised in A* algorithm; the node traversed previously to
	///        reach this node.
	//-------------------------------------------------------------------
	iAIPathNode* mParent;

	//-------------------------------------------------------------------
	/// @var bool mOpen
	/// @brief Utilised in A* algorithm; the node requires checking.
	//-------------------------------------------------------------------
	bool mOpen;

	//-------------------------------------------------------------------
	/// @var bool mClosed
	/// @brief Utilised in A* algorithm; if node has been checked, set as
	///        closed.
	//-------------------------------------------------------------------
	bool mClosed;


	//-------------------------------------------------------------------
	// XXTH
	/// @var bool mBadNode 
	/// @brief marked as Badnode used to remove a node on grid creation
	//-------------------------------------------------------------------
	bool mBadNode;

   //-------------------------------------------------------------------
   // XXTH
   //-------------------------------------------------------------------
   Point3F getPosition() { return mPosition; }

};

#endif
