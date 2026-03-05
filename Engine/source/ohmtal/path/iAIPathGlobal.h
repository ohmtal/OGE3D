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
/// @file iAIPathGlobal.h
//-------------------------------------------------------------------
/// @class iAIPathGlobal
/// @author Gavin Bunney
/// @version 1.0
/// @brief Global macros for the Seek :: Path functions.
/// 
/// Holds a collection of the macros used within the various path
/// finding classes.
//-------------------------------------------------------------------
#ifndef _IAIPATHGLOBAL_H_
#define _IAIPATHGLOBAL_H_

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_COLLISION_MASK
/// @brief Collision mask to detect for node clearance.
//-------------------------------------------------------------------
//#define IAIPATHGLOBAL_SMOTH_COLLISION_MASK (InteriorObjectType | StaticShapeObjectType | VehicleObjectType | PlayerObjectType | StaticTSObjectType | TerrainObjectType)

//#define IAIPATHGLOBAL_SMOTH_COLLISION_MASK (InteriorObjectType | StaticShapeObjectType |  StaticTSObjectType | TerrainObjectType)
#define IAIPATHGLOBAL_COLLISION_MASK ( StaticObjectType | StaticShapeObjectType )
#define IAIPATHGLOBAL_CUSTIOM_COLLISION_MASK (StaticObjectType | StaticShapeObjectType | TerrainObjectType | TerrainLikeObjectType  | InteriorLikeObjectType )


//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_MAX_SLOPE
/// @brief Max slope between two nodes.
//-------------------------------------------------------------------
//XXTH disabled !!
#define IAIPATHGLOBAL_MAX_SLOPE					60.0f

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_MAX_SMOOTHED_SLOPE
/// @brief Max slope between path nodes for smoothing.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_MAX_SMOOTHED_SLOPE		60.0f

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_NODE_CLEARANCE
/// @brief Amount of clearance in X, Y & Z around a node.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_NODE_CLEARANCE			Point3F(1.5f, 1.5f, 3.f) //XXTH orig: 2.3f)



//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL
/// @brief MoveModifier for a node to be considered untraversal.
//-------------------------------------------------------------------
//move modifier U8!
#define IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL	255

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_MOVE_MODIFIER_WATER
/// @brief MoveModifier when a node is in water.
//-------------------------------------------------------------------
//#define IAIPATHGLOBAL_MOVE_MODIFIER_WATER		70.0f

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_BUFFER_INTERIOR
/// @brief Amount of clearance around an interior for a grid.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_BUFFER_INTERIOR		Point3F(10.0, 10.0, 0)

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_DENSITY_INTERIOR
/// @brief Density of nodes within an interior.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_DENSITY_INTERIOR		2.0f

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_DENSITY_TERRAIN
/// @brief Density of nodes on normal terrain.
//-------------------------------------------------------------------
//#define IAIPATHGLOBAL_GRID_DENSITY_TERRAIN		0.1f
#define IAIPATHGLOBAL_GRID_DENSITY_TERRAIN		0.4f

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_RENDER_CLEARANCE
/// @brief Clearance above node position to render the grid.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_RENDER_CLEARANCE		Point3F(0, 0, 0.3f)

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_RENDER_NODE_HEIGHT
/// @brief Height of rendered nodes.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_RENDER_NODE_HEIGHT	Point3F(0, 0, 1.0f)

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_RENDER_COLOUR
/// @brief Colour of the rendered grid: R, G, B, Alpha
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_RENDER_COLOUR		31, 102, 155, 255

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_RENDER_BOX_COLOUR
/// @brief Colour of the rendered grid bounding box: R, G, B, Alpha
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_RENDER_BOX_COLOUR	255, 0, 0, 255

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_GRID_RENDER_NODE_COLOUR
/// @brief Colour of the rendered nodes on a grid: R, G, B, Alpha
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_GRID_RENDER_NODE_COLOUR	255, 255, 0, 255

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_PATH_RENDER_CLEARANCE
/// @brief Clearance above node to render the path.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_PATH_RENDER_CLEARANCE		Point3F(0, 0, 1.0f)

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_PATH_RENDER_NODE_HEIGHT
/// @brief Height of rendered path nodes.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_PATH_RENDER_NODE_HEIGHT	Point3F(0, 0, 1.0f)

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_PATH_RETRY_COUNT
/// @brief Number of retries to perform if a path is not found.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_PATH_RETRY_COUNT			0 //XXTH was 2 !

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_PATH_SMOOTH_ANGLE_THRESHOLD
/// @brief Angle (degrees) between two points to detect if it can
///        be removed in the path smoothing process. The angle
///        between the two points must be +/- threadhold around 90.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_PATH_SMOOTH_ANGLE_THRESHOLD	15.0

//-------------------------------------------------------------------
/// @def IAIPATHGLOBAL_INVALID_POSITION
/// @brief Used to detect for invalid position nodes.
//-------------------------------------------------------------------
#define IAIPATHGLOBAL_INVALID_POSITION			Point3F(0.f,0.f,0.f)


#endif
