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
/// @file iAIPath.h
//-------------------------------------------------------------------
/// @class iAIPath
/// @author Gavin Bunney
/// @version 1.0
/// @brief Represents a path from one point to another.
/// 
/// The iAIPath class holds a list of all nodes (positions) from one
/// start position to another end position.
/// <br><br>
/// TypeMask |= iAIPathObjectType
//-------------------------------------------------------------------
#ifndef _IAIPATH_H_
#define _IAIPATH_H_

#include "iAIPathNode.h"
#include "iAIPathMap.h"
#include "scene/sceneObject.h"

class iAIPath : public SceneObject
{
	typedef SceneObject Parent;

	friend class iAIAgent;


public:

	//-------------------------------------------------------------------
	/// @var DECLARE_CONOBJECT(iAIPath)
	/// @brief TorqueScript object.
	//-------------------------------------------------------------------
	DECLARE_CONOBJECT(iAIPath);

	//-------------------------------------------------------------------
	/// @fn iAIPath() 
	/// @brief Default constructor.
	//-------------------------------------------------------------------
	iAIPath();

	//-------------------------------------------------------------------
	/// @fn bool createPath(iAIPathMap* pathMap,
	///                     const Point3F start, const Point3F end,
	///                     const bool smoothPath = true)
	/// @brief Creates a path from the start node to the end node.
	///
	/// @param pathMap Pointer to the pathmap to generate path within.
	/// @param start Point to start the path from.
	/// @param end Point to end the path at.
	/// @param smoothPath Flag to smooth the path. Default true.
	/// @return Path creation success.
	//-------------------------------------------------------------------
	bool createPath(iAIPathMap* pathMap, Point3F start, Point3F end, const bool smoothPath = true);

	//-------------------------------------------------------------------
	/// @fn Point3F getNextPosition()
	/// @brief Retrieves the next position to goto and removes the last
	///        visited node.
	/// 
	/// @return Point3F of next node on the path.
	//-------------------------------------------------------------------
	Point3F getNextPosition();

	//XXTH current move modifier from last getNextPosition :D
	U8 mCurMoveModifier;

    // XXTH getPosition(S32 idx) same as getNextPosition but does NOT delete the Position!!
    Point3F getNodePosition(S32 idx);


	//-------------------------------------------------------------------
	/// @fn bool hasNextNode()
	/// @brief Test to see if this path has another node or not.
	/// 
	/// @return True if the path has another node.
	//-------------------------------------------------------------------
	bool hasNextNode();

	//-------------------------------------------------------------------
	/// @fn U32 nodeCount()
	/// @brief Retrieve the number of nodes left in the path.
	/// 
	/// @return U32 Number of nodes left in the path.
	//-------------------------------------------------------------------
	U32 nodeCount();

	//-------------------------------------------------------------------
	/// @fn bool onAdd()
	/// @brief Called on adding to Sim.
	//-------------------------------------------------------------------
	bool onAdd();

	//-------------------------------------------------------------------
	/// @fn bool onRemove()
	/// @brief Called on removal from Sim.
	//-------------------------------------------------------------------
	void onRemove();

	//-------------------------------------------------------------------
	/// @fn bool prepRenderImage(SceneState *state, const U32 stateKey,
	///     const U32 startZone, const bool modifyBaseZoneState = false)
	/// @brief Called on scene render. Detects if the path needs to be
	///        rendered, calls renderObject if need to render the path.
	///
	/// @param state the current SceneState.
	/// @param stateKey key for the state.
	/// @param startZone zone for scene start.
	/// @param modifyBaseZoneState flag to modify the base zone state.
	/// @return scene state change.
	//-------------------------------------------------------------------
	//bool prepRenderImage(SceneRenderState *state, const U32 stateKey, const U32 startZone, const bool modifyBaseZoneState = false);
   virtual void prepRenderImage(SceneRenderState* state);


	//-------------------------------------------------------------------
	/// @fn void renderObject(SceneState *state, SceneRenderImage *image)
	/// @brief Renders the path.
	///
	/// @param state the current SceneState.
	/// @param image scene to render in.
	//-------------------------------------------------------------------
	//void renderObject(SceneRenderState*state, SceneRenderImage *image);
   void renderObject(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* mi);

	//-------------------------------------------------------------------
	/// @fn static void initPersistFields()
	/// @brief Exposes some variables to TorqueScript.
	//-------------------------------------------------------------------
	static void initPersistFields();

protected:

	//-------------------------------------------------------------------
	/// @fn void updateWorldBox()
	/// @brief Repositions and resizes the worldbox. Required to ensure
	///        that path is rendered in the scene.
	//-------------------------------------------------------------------
	void updateWorldBox();

	//-------------------------------------------------------------------
	/// @var bool mTraversing
	/// @brief Used to set a flag if the GetNextNode has been called
	///        previously or not.
	//-------------------------------------------------------------------
	bool mTraversing;

	//-------------------------------------------------------------------
	/// @var bool mShow
	/// @brief Render flag to show the path on scene pass.
	//-------------------------------------------------------------------
	bool mShow;

	//-------------------------------------------------------------------
	/// @var bool mRenderSpline
	/// @brief Render flag to render as a spline path. If set to false,
	///        path will render linear.
	//-------------------------------------------------------------------
	bool mRenderSpline;

	//-------------------------------------------------------------------
	/// @var ColorI mPathColour
	/// @brief Colour of the rendered path.
	//-------------------------------------------------------------------
	ColorI mPathColour;

	//-------------------------------------------------------------------
	/// @var ColorI mPathNodeColour
	/// @brief Colour of the nodes on the rendered path.
	//-------------------------------------------------------------------
	ColorI mPathNodeColour;

	//-------------------------------------------------------------------
	/// @var Vector<iAIPathNode*> mPathNodes
	/// @brief Vector of all nodes in the path.
	//-------------------------------------------------------------------
	Vector<iAIPathNode*> mPathNodes;

	//-------------------------------------------------------------------
	/// @var iAIPathNode* mLastNode
	/// @brief Pointer to the last node which was returned.
	//-------------------------------------------------------------------
	iAIPathNode* mLastNode;
};

#endif
