//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// BasicGrid
// 2020-03-16 port to t3d
//-----------------------------------------------------------------------------

#ifndef _BASICGRID_H_
#define _BASICGRID_H_

//-------------------------------------------
#ifndef _TYPES_H_
#include "platform/types.h"
#endif
#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
//-------------------------------------------

#define BASICGRID_UNWALKABLE  255

class BasicGrid;

class BasicGridNode
{
private:
   Point3F mPos;
   Point3F mCenterPos;
   VectorF mNormal;
   U32  mFlags; 
   U8  mWeight;  // 0 = unwalkable!
   S32 mIntValues[10];
   bool mDirty;  // for client/server used in terraingrid for flags only!

public:
	//pathfinding stuff:
	bool mOpen;
	bool mClosed;
	F32 mFitness;
	F32 mLowestCostFromStart;
	F32 mHeuristicCostToGoal;
	BasicGridNode* mParent;
   BasicGrid* mGrid;


public:
   void setPos(Point3F lPos);
   void setZ(F32 value) {
      mPos.z = value;
      mCenterPos.z = value;
   }

   void setNormal(VectorF value) {
      mNormal = value;
   }
   VectorF getNormal() { return mNormal; }

   Point3F getPos() { return mPos;}
   Point3F getCenterPos() {
      return mCenterPos;
   }
   Point2I getPos2I() { return Point2I((U32)mPos.x, (U32)mPos.y); }
   Point2I getCenterPos2I() { return Point2I((U32)mCenterPos.x, (U32)mCenterPos.y);  }

   void setIntValue(S32 idx,S32 lValue){ 
      if (idx < 10) {
         mIntValues[idx] = lValue;
      }
   }
   S32 getIntValue(S32 idx){ 
	   if (idx<10) 
		   return mIntValues[idx]; 
	   else 
		   return 0; 
   }

   void setWeight (S32 lValue) { mWeight = (U8)lValue; }
   S32 getWeight () { return (S32)mWeight; }

   bool isDirty() { return mDirty; }
   void setDirty(); // { mDirty = true; if (mGrid) mGrid->setDirty(); }
   void setClean() { mDirty = false; }

   void setFlags (U32 lFlags) {
      mFlags = lFlags;
      setDirty();
   }
   U32 getFlags () { return mFlags; }

   bool isFlagOn(U8 lFlag)  { 
	   return (mFlags & BIT(lFlag)) == BIT(lFlag); 
   }
   void toggleFlag(U8 lFlag) { 
	   if (!isFlagOn(lFlag)) 
		   mFlags |= BIT(lFlag); 
	   else
		   mFlags ^= BIT(lFlag);
      setDirty();
   }

   void addFlag(U8 lFlag) { 
	   if (!isFlagOn(lFlag)) 
		   mFlags |= BIT(lFlag);
      setDirty();
   }
   void rmvFlag(U8 lFlag) { 
	   if (isFlagOn(lFlag))  
			mFlags ^= BIT(lFlag);
      setDirty();
   }



   U8 getPathWeight()
   {
	   if (isFlagOn(0))
		   return 255;
		return mWeight;
   }



   BasicGridNode() {
	   mPos = mCenterPos = mNormal =  Point3F(0.f,0.f,0.f);
	   mFlags=0;
      mDirty = false;
	   mOpen    = false; 
	   mClosed  = false;
	   mFitness = 0.f;
	   mWeight = 0; //was 128 ?!
	   mLowestCostFromStart = 0.f;
	   mHeuristicCostToGoal = 0.f;
	   mParent = NULL;
      mGrid = NULL;
	   for (S32 i=0;i<10; i++)
		   mIntValues[i]=0; 
   }

   ~BasicGridNode() {
   }
};


class BasicGrid : public SimObject //SceneObject
{
   typedef SimObject Parent;

public:
	BasicGrid();
	~BasicGrid();
	DECLARE_CONOBJECT(BasicGrid);

private:
	RectI mArea;
	F32 mSquareSize;
   F32 mHalfSquareSize;
	bool mInitDone;
	bool mDebugGrid;
    bool mClientGame;

	S32 mNodesX,mNodesY, mNodeCount;
	BasicGridNode* mNodes;

   bool mDirty; //used for rendering 

	void resetNodeVariables(Vector<BasicGridNode*> &affectedList);
	F32 estimateCostToGoal(BasicGridNode* from, BasicGridNode* goal);


public:
   bool onAdd();
   void onRemove();

   bool generatePath(BasicGridNode* startNode, BasicGridNode* goalNode, Vector<BasicGridNode*> &replyList, const bool smoothPath = true);
   SimObject * createPath(Point3F start, Point3F end, const bool smoothPath = true);


   BasicGridNode* getNeighbour(BasicGridNode* startNode, U8 direction, S32& nodeIndex);

   S32  getNodeIndex(F32 x, F32 y, bool lPrec = false); //lPrec = more precise by float params, but slower

   bool getNodesByRect(const RectF &lRect, Vector<S32> &lList, bool lCanOverlap = false);
   

   bool getClientGame() { return mClientGame; }
   void setClientGame(bool lValue) { mClientGame= lValue; }

   void setSquareSize(F32 value) { mSquareSize = value; mHalfSquareSize = value / 2.f; }
   F32 getSquareSize() { return mSquareSize; }
   F32 getHalfSquareSize() { return mHalfSquareSize; }

   void  setArea(RectI lArea) { mArea = lArea; }
   RectI getArea() { return mArea; }

   bool getDebugGrid() { return mDebugGrid; }
   void setDebugGrid(bool lValue) { mDebugGrid = lValue; }
   BasicGridNode* findNode(F32 x, F32 y);
   BasicGridNode* findNode(F32 x, F32 y, S32& nodeIndex);

   BasicGridNode* getNodeById(S32 lId) { 
	   if (lId >=0 && lId < getNodeCount())
			return &mNodes[lId]; 
	   else
		   return NULL;
   }

   U32 getNodeCount() { return mNodeCount; }
   virtual void init(RectI lArea, F32 lSquareSize = 8);
   
   void setDirty() { mDirty = true; }
   void setClean() { mDirty = false; }
   bool getDirty() { return mDirty; }

   void PrintInfo(bool listNodes = false);

   bool getInitDone() { return mInitDone; }
   
};

#endif
