//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
// Copyright (c) 2023 Ohmtal Game Studio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
//
//
// class aiEnhancedPlayer 
// 
// @desc Like aiPlayer but less collision to prevent stuck and callbacks on player class
//
// @created 2023-02-16
// @author  T.Huehn XXTH
//
// For humancontrolled i use the existing flag: mIsAiControlled
// 
//-----------------------------------------------------------------------------
#ifndef _AIENHANCEDPLAYER_H_
#define _AIENHANCEDPLAYER_H_

#ifndef _ENHANCEDPLAYER_H_
#include "enhancedPlayer.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif

#ifdef TORQUE_NAVIGATION_ENABLED
#include "navigation/navPath.h"
#include "navigation/navMesh.h"
#include "navigation/coverPoint.h"
#endif // TORQUE_NAVIGATION_ENABLED


class aiEnhancedPlayer : public enhancedPlayer
{
protected:
   typedef enhancedPlayer Parent;

public:
   enum MoveState {
      ModeStop,                       // AI has stopped moving.
      ModeMove,                       // AI is currently moving.
      ModeStuck,                      // AI is stuck, but wants to move.
      ModeSlowing,                    // AI is slowing down as it reaches it's destination.
   };

   //XXTH 
   enum StrafeMode {
      left,
      right,
      back,
      forward
   };
private:
   MoveState _mMoveState;
   F32 mMoveSpeed;
   F32 mMoveTolerance;                 // Distance from destination before we stop
   F32 mAttackRadius;                  // Distance to trigger weaponry calcs
   Point3F mMoveDestination;           // Destination for movement
   Point3F mLastLocation;              // For stuck check
   F32 mMoveStuckTolerance;            // Distance tolerance on stuck check
   S32 mMoveStuckTestDelay;            // The number of ticks to wait before checking if the AI is stuck
   S32 mMoveStuckTestCountdown;        // The current countdown until at AI starts to check if it is stuck
   bool mMoveSlowdown;                 // Slowdown as we near the destination

   SimObjectPtr<GameBase> mAimObject; // Object to point at, overrides location
   bool mAimLocationSet;               // Has an aim location been set?
   Point3F mAimLocation;               // Point to look at
   bool mTargetInLOS;                  // Is target object visible?

   bool mStrafeMode;                   // we strafe so no rotation

   Point3F mAimOffset;

   // move triggers
   bool mMoveTriggers[MaxTriggerKeys];

   // Utility Methods (XXTH made it virtual) 
   virtual void throwCallback(const char* name);

#ifdef TORQUE_NAVIGATION_ENABLED
   /// Should we jump?
   enum JumpStates {
      None,  ///< No, don't jump.
      Now,   ///< Jump immediately.
      Ledge, ///< Jump when we walk off a ledge.
   } mJump;

   /// Stores information about a path.
   struct PathData {
      /// Pointer to path object.
      SimObjectPtr<NavPath> path;
      /// Do we own our path? If so, we will delete it when finished.
      bool owned;
      /// Path node we're at.
      U32 index;
      /// Default constructor.
      PathData() : path(NULL)
      {
         owned = false;
         index = 0;
      }
   };

   /// Path we are currently following.
   PathData mPathData;


   /// Get the current path we're following.
   NavPath* getPath() { return mPathData.path; }

   /// Stores information about our cover.
   struct CoverData {
      /// Pointer to a cover point.
      SimObjectPtr<CoverPoint> cover;
      /// Default constructor.
      CoverData() : cover(NULL) {}
   };

   /// Current cover we're trying to get to.
   CoverData mCoverData;


   /// Information about a target we're following.
   struct FollowData {
      /// Object to follow.
      SimObjectPtr<SceneObject> object;
      /// Distance at whcih to follow.
      F32 radius;
      Point3F lastPos;
      /// Default constructor.
      FollowData() : object(NULL)
      {
         radius = 5.0f;
         lastPos = Point3F::Zero;
      }
   };

   /// Current object we're following.
   FollowData mFollowData;


   /// NavMesh we pathfind on.
   SimObjectPtr<NavMesh> mNavMesh;

   /// Move to the specified node in the current path.
   void moveToNode(S32 node);
#endif // TORQUE_NAVIGATION_ENABLED
protected:
   virtual void onReachDestination();
   virtual void onStuck();


public:
   DECLARE_CONOBJECT(aiEnhancedPlayer);


   aiEnhancedPlayer():
      mIsSimPathWalking(false)
      ,mSimPathReverse(false)
      ,mSimPathObject(NULL)
      ,mCurSimPathNodeIndex(-1)
      ,mMoveDestination(0.0f, 0.0f, 0.0f)
      ,mMoveSpeed(1.0f)
      ,mMoveTolerance(0.25f)
      ,mMoveStuckTolerance(0.01f)
      ,mMoveStuckTestDelay(30)
      ,mMoveStuckTestCountdown(0)
      ,mMoveSlowdown(true)
      ,_mMoveState(ModeStop)
      ,mMoveState_saved(-1) 
      ,mAimObject(0)
      ,mAimLocationSet(false)
      ,mTargetInLOS(false)
      ,mAimOffset(0.0f, 0.0f, 0.0f)
#ifdef TORQUE_NAVIGATION_ENABLED
      ,mJump(None)
      ,mNavSize(Regular)
      ,mLinkTypes(AllFlags)
#endif
      
      ,mStrafeMode(false)
   {
      for (S32 i = 0; i < MaxTriggerKeys; i++)
         mMoveTriggers[i] = false;

      mIsAiControlled = true;
      setMoveTolerance(1.f);

      mCollisionMoveMask = (
         TerrainObjectType
         | TerrainLikeObjectType
         // | InteriorLikeObjectType
         | WaterObjectType
         // looks better but they stuck !!
         //why do they stop ?
         // ok finally off!! | PlayerObjectType      
         | StaticShapeObjectType
         // | VehicleObjectType 
         | PhysicalZoneObjectType
         );



      mServerCollisionContactMask = (mCollisionMoveMask |
         (ItemObjectType |
            TriggerObjectType |
            CorpseObjectType
            ));

      mClientCollisionContactMask = mCollisionMoveMask | PhysicalZoneObjectType;

   } //constuctor

   static void initPersistFields();

   bool onAdd();
   void onRemove();

   virtual bool getAIMove(Move* movePtr);
   virtual void updateMove(const Move* move);
   /// Clear out the current path.
   void clearPath();
   /// Stop searching for cover.
   void clearCover();
   /// Stop following me!
   void clearFollow();

   // Targeting and aiming sets/gets
   void setAimObject(GameBase* targetObject);
   void setAimObject(GameBase* targetObject, const Point3F& offset);
   GameBase* getAimObject() const { return mAimObject; }
   void setAimLocation(const Point3F& location);
   Point3F getAimLocation() const { return mAimLocation; }
   void clearAim();
   void getMuzzleVector(U32 imageSlot, VectorF* vec);
   bool checkInLos(GameBase* target = NULL, bool _useMuzzle = false, bool _checkEnabled = false);
   bool checkInFoV(GameBase* target = NULL, F32 camFov = 45.0f, bool _checkEnabled = false);
   F32 getTargetDistance(GameBase* target, bool _checkEnabled);

   // Movement sets/gets
   void setMoveSpeed(const F32 speed);
   F32 getMoveSpeed() const { return mMoveSpeed; }
   void setMoveTolerance(const F32 tolerance);
   F32 getMoveTolerance() const { return mMoveTolerance; }
   void setMoveDestination(const Point3F& location, bool slowdown);
   Point3F getMoveDestination() const { return mMoveDestination; }
   void stopMove();

   //XXTH
   void setMoveState(MoveState lState) {
      _mMoveState = lState;
   }
   MoveState getMoveState() {
      return _mMoveState;
   }
   bool havePath() const { return !mPathData.path.isNull() && mPathData.owned && mPathData.path->size() >0 ; }
   //XXTH <<<<

   void setAiPose(S32 pose);
   S32  getAiPose();

   // Trigger sets/gets
   void setMoveTrigger(U32 slot, const bool isSet = true);
   bool getMoveTrigger(U32 slot) const;
   void clearMoveTriggers();

#ifdef TORQUE_NAVIGATION_ENABLED
   /// @name Pathfinding
   /// @{

   enum NavSize {
      Small,
      Regular,
      Large
   } mNavSize;
   void setNavSize(NavSize size) { mNavSize = size; updateNavMesh(); }
   NavSize getNavSize() const { return mNavSize; }

   bool setPathDestination(const Point3F& pos);
   Point3F getPathDestination() const;

   void followNavPath(NavPath* path);
   void followObject(SceneObject* obj, F32 radius);

   void repath();

   bool findCover(const Point3F& from, F32 radius);

   NavMesh* findNavMesh() const;
   void updateNavMesh();
   NavMesh* getNavMesh() const { return mNavMesh; }

   /// Get cover we are moving to.
   CoverPoint* getCover() { return mCoverData.cover; }

   /// Types of link we can use.
   LinkData mLinkTypes;

   /// @}
#endif // TORQUE_NAVIGATION_ENABLED
   // New method, restartMove(), restores the AIPlayer to its normal move-state
   // following animation overrides from AFX. The tag argument is used to match
   // the latest override and prevents interruption of overlapping animation
   // overrides.
   // New method, saveMoveState(), stores the current movement state
   // so that it can be restored when restartMove() is called.
   // See related anim-clip changes in Player.[h,cc].
private:
   S32 mMoveState_saved;
public:
   void restartMove(U32 tag);
   void saveMoveState();

   //XXTH strafeStep
   virtual void strafeStep(aiEnhancedPlayer::StrafeMode lMode, F32 distance);

   bool  getAimObjectInLos() { return mTargetInLOS; }



protected:


   bool  mIsSimPathWalking;
   bool  mSimPathReverse; 
   SimPath::Path* mSimPathObject;
   S32   mCurSimPathNodeIndex;
   bool MoveToSimPathWayMarker(Marker* lMarker);
   bool  MoveToNextSimPathWayPoint();

   

public:
   Marker* getClosestSimPathMarker();
   void setSimPathObject(SimPath::Path* lPath);
   bool getIsSimPathWalking() { return mIsSimPathWalking; }
   bool startSimPath(bool reverse);
   void stopSimPath();

};

#endif // _ENHANCEDPLAYER_H_
