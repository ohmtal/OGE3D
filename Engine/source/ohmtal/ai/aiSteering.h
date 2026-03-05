//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// AISteering by T.Huehn 2009 (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------
/*
  Engine changes:
   1.) gameBase.h
      After "virtual void preprocessMove(Move *move) {}"  add:
     //XXTH Virtual for AISteering!
       virtual F32 getMaxForwardVelocity() { return 0.f; }
   2.) mCostants.h
     add:
	  #define M_RAD        0.017453292519943295769236907684886

  Notes:
    1.) getHideForce needs a simgroup of Obstancles to find the best hiding position, 
		this is not the very best implementation. It's not very usable for 
		let's say shapereplicator and its very slow on many objects in that 
		group. So its more an example implementation than a useable steering.

	

*/


#ifndef _AISTEERING_H_
#define _AISTEERING_H_

#ifndef _AIGLOBALS_H_
#include "aiGlobals.h"
#endif


#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif
#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

#ifdef AI_USEPATH
#ifdef AI_NAVIGATION
#include "navigation/navPath.h"
#include "navigation/navMesh.h"
#include "navigation/coverPoint.h"
#endif  //AI_NAVIGATION
//iAIPath navigation
#ifndef _IAIPATH_H_
#include "ohmtal/path/iAIPath.h"
#endif

#ifndef _IAIPATHMAP_H_
#include "ohmtal/path/iAIPathMap.h"
#endif

#endif  //AI_USEPATH

#ifndef _AIPATH_H_
#include "aiPath.h"
#endif

	enum SteeringBehaviourTypes {
		NoneBehaviourType					= BIT(0)
		,SeekBehaviourType					= BIT(1)
		,FleeBehaviourType					= BIT(2)
		,ArriveBehaviourType				= BIT(3)
		,WanderBehaviourType				= BIT(4)
		,CohesionBehaviourType				= BIT(5)
		,SeparationBehaviourType			= BIT(6)
		,AlignmentBehaviourType				= BIT(7)
		,unused								= BIT(8)
		,Wall_AvoidanceBehaviourType		= BIT(9)
		,Follow_PathBehaviourType			= BIT(10)
		,PursuitBehaviourType				= BIT(11)
		,EvadeBehaviourType					= BIT(12)
		,InterPoseBehaviourType				= BIT(13)
		,HideBehaviourType					= BIT(14)
		,FlockBehaviourType					= BIT(15)
		,Offset_PursuitBehaviourType		= BIT(16)
		,PursuitPathBehaviourType			= BIT(17)
	};

	enum SteeringDeceleration{slow = 3, normal = 2, fast = 1};


class AISteering {
	

private:
      SimObjectPtr<GameBase> mOwner; // Object to point at, overrides location
	  F32 mMoveTolerance;
	  
	  U32 mWallAvoidMask;
	  F32  mFeelerLen;

	  U32 mGroundMask;

  	  AIPath*     mPath; 
	  bool mUseSplinePath; 
	  F32 mPathSplinePrec; //max 0.5!!
#ifdef AI_USEPATH
#ifdef AI_NAVIGATION
     //SimObjectPtr<NavMesh> mNavMesh; //FIXME multiple meshes
     SimObjectPtr<NavMesh> mNavMesh;
     SimObjectPtr<NavPath> mPtrNavPath;
     NavMesh* getNavMesh() { return mNavMesh; }
     bool mNavigationUnfinished; 
     bool planNavigationPath(Point3F lTo);
#else
	  iAIPathMap*  mPathMap;
#endif //AI_NAVIGATION
#endif //AI_USEPATH
	  VectorF mLastPathDestination;
	  VectorF mLastPathStart;

	  VectorF mWanderTarget;

	  //used by getHideForce
	  VectorF getHidingPosition( VectorF lPosOb, F32 lRadiusOb, VectorF lPosTarget);

      bool mPathMapInitDone;
#ifdef AI_USEPATH
	  void initPathMap();
#endif
public:
	  AISteering(GameBase* lOwner);
      ~AISteering();


	  GameBase* getOwner() { return mOwner; }
	  void setMoveTolerance(F32 lTol) { mMoveTolerance=lTol; }
	  F32 getMoveTolerance() { return mMoveTolerance; }

	  AIPath* getPath() { return mPath; }

	  void throwOwnerCallback( const char *name);

	  void setUseSplinePath(bool lValue) { mUseSplinePath = lValue; }
	  bool getUseSplinePath() { return mUseSplinePath; }
	  void setPathSplinePrec(F32 lValue) { mPathSplinePrec = mClampF(lValue,0.01f,0.5f); }
	  F32  getPathSplinePrec() { return mPathSplinePrec; }

	  void resetWander() { mWanderTarget.set(0.f,0.f,0.f); };
	  VectorF getHeading( GameBase *lObj );
	  VectorF getHeading2D( GameBase *lObj );
	  F32 getYawToDestination( VectorF lDestPos, F32 lMaxRad);
	  VectorF getPointToWorldSpace(Point3F lPoint, MatrixF lTransform);

	  // void setBehaviourTypeMask(U32 lMask) { mBehaviourTypeMask = lMask; }
	  
	  VectorF getSeekForce(VectorF lTargetPos, bool lArriveCallBack = false); 
	  VectorF getFleeForce(VectorF lTargetPos, F32 lPanicDistance=20.f); 
	  VectorF getArriveForce(VectorF lTargetPos, SteeringDeceleration lDeceleration = normal); 
	  VectorF getPursuitForce( GameBase *lEvader, F32 lStopDistance=0.f);
     VectorF getPursuitVector(GameBase* lEvader, F32 lStopDistance = 0.f); //bullshit , bool useStopDistanceForCloseTo = false);
	  VectorF getEvadeForce( GameBase *lPursuer, F32 lPanicDistance=20.f);
	  VectorF getEvadeVector( GameBase *lPursuer, F32 lPanicDistance=20.f);
	  VectorF getWanderForce( F32 lRad = 2.2f, F32 lDist=10.f, F32 lJitter=1.f);
	  VectorF getWallForce( bool lStuck = false);
	  VectorF getInterPoseForce( GameBase *lAgentA, GameBase *lAgentB);
	  VectorF getHideForce(GameBase *lTarget, SimGroup *lObstGroup, F32 mMinDistance = 15.f, F32 mMaxDistance = 30.f);
	  VectorF getPathForce( );
	  VectorF getOffsetPursuitForce(GameBase *lLeader,VectorF lOffset);

	  VectorF getSeparationForce(F32 lRadius, const SimpleQueryList	*lNeighbours, S32 lIgnorePlayerType = -1 );
	  VectorF getAlignmentForce(F32 lRadius, const SimpleQueryList	*lNeighbours);
	  VectorF getCohesionForce(F32 lRadius, const SimpleQueryList	*lNeighbours);
	  
     bool clearPathObject();

     bool setPathObject( SimPath::Path *pathObject , bool lLooping = true, bool reverse = false);
      bool setIAIPath( iAIPath* pathObject );

      AIPath* getPathObject() { return mPath; }
#ifdef AI_USEPATH
	  bool updatePursuitPath( GameBase *lEvader );
	  bool setPathDestination( Point3F lTo, bool lForce = false);
#endif
	  bool stopPath();
	  bool resetPath();
	  bool hasPath() { return mPath->initDone(); }
	  bool continuePath();
	  Point3F* getClosestWayPoint(Point3F lFrom);

	  
};




#endif // _AISTEERING_H_
