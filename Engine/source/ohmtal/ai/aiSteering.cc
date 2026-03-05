//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// AISteering by T.Huehn 2009 (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------
#include "aiSteering.h"
#include "aiMath.h"

#include "core/util/safeDelete.h"

#ifdef AI_USEPATH

#ifdef AI_NAVIGATION
#include "navigation/navPath.h"
#include "navigation/navMesh.h"
#include "navigation/coverPoint.h"
#endif 

//iAIPath navigation
#include "ohmtal/path/iAIPath.h"
#include "ohmtal/path/iAIPathMap.h"
#endif

// --------------------------------------------------------------------------------------------
AISteering::AISteering(GameBase* lOwner) 
{
	mOwner = lOwner;
	mMoveTolerance = 0.5f; //0.25f;

	mWallAvoidMask =
      StaticShapeObjectType
      |  StaticObjectType
      | TerrainObjectType
      //is a player sucks!!       | DynamicShapeObjectType
      ;
//TGE:	mWallAvoidMask = InteriorObjectType | StaticShapeObjectType  | StaticTSObjectType | StaticObjectType | TerrainObjectType;

	mFeelerLen =  getOwner()->getObjBox().len_y() / 2.f  + 1.75f; //2.1 was +1.25f changed to 1.75f

	mGroundMask =
      TerrainObjectType
      | StaticObjectType
      | StaticShapeObjectType
      ;
//TGE:	mGroundMask = TerrainObjectType | InteriorObjectType | StaticObjectType | StaticShapeObjectType | StaticTSObjectType;




    // Path
    mPath = new AIPath();
    mUseSplinePath = true;
    mPathSplinePrec = 0.1f;
	mLastPathDestination = Point3F (99999999.991f,99999999.991f,99999999.991f);

#ifdef AI_USEPATH
#ifdef AI_NAVIGATION
   mNavMesh = NULL;
   mPtrNavPath = NULL;
   mNavigationUnfinished = false;
#else
	mPathMap = NULL;
#endif
#endif //AI_USEPATH
	mPathMapInitDone = false;

	resetWander();
}
AISteering::~AISteering()
{

#ifdef AI_USEPATH
#ifdef AI_NAVIGATION
   if (!mPtrNavPath.isNull())
      mPtrNavPath->deleteObject();
#endif
#endif //AI_USEPATH

	SAFE_DELETE(mPath);

}
// --------------------------------------------------------------------------------------------
// Steering
// --------------------------------------------------------------------------------------------
VectorF AISteering::getHeading(GameBase *lObj ) {
	VectorF lHeading;
	lObj->getTransform().getColumn(1,&lHeading);
	return lHeading;
}

VectorF AISteering::getHeading2D(GameBase *lObj ) {
	VectorF lHeading;
	lObj->getTransform().getColumn(1,&lHeading);
	return AIMath::Vector2D(lHeading);
}

// --------------------------------------------------------------------------------------------
F32 AISteering::getYawToDestination( VectorF lDestPos, F32 lMaxRad)
{
   F32 result = 0;
    // 1. get Diff to targetPos
   VectorF lVecDiff = AIMath::Vector2D(lDestPos - getOwner()->getPosition());
	if (lVecDiff.isZero()) { //exact same position ?! bad!
	  return(0.f);
	}

	lVecDiff.normalize();
   //2. get heading !!is normalized ?!
 	 VectorF lHeading = getHeading2D(getOwner());
   // 3. determine the angle between the heading vector and the target, clamp for safety
	 F32 lDot = mClampF(mDot(lHeading,lVecDiff),-1.f,1.f); 
     result = mAcos(lDot);
	 if (result < 0.00001f) 
		 return 0.f;
     if (result > lMaxRad) 
		result = lMaxRad;

	 result *= AIMath::Sign2D(lHeading,lVecDiff);

  return (result);

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getPointToWorldSpace(Point3F lPoint, MatrixF lTransform)
{
 //project the target into world space
  VectorF lResult;
  lResult.zero();
  MatrixF matTransform = lTransform;
  matTransform.mulP(lPoint, &lResult);

  return lResult;
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getSeekForce(VectorF lTargetPos, bool lArriveCallBack ) 
{

  VectorF lDesiredVelocity = AIMath::Vector2D(lTargetPos - getOwner()->getPosition());

  
  if (mMoveTolerance > lDesiredVelocity.len()) {
	  if (lArriveCallBack) 
		throwOwnerCallback("onSeekDestination");
	  lDesiredVelocity.zero();
	  return (lDesiredVelocity);
  }
  lDesiredVelocity.normalize();;
  lDesiredVelocity *= getOwner()->getMaxForwardVelocity();
//  Con::printf("MAXSPEED:%f",getOwner()->getMaxForwardVelocity());
  return (lDesiredVelocity - getOwner()->getVelocity());
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getFleeForce(VectorF lTargetPos, F32 lPanicDistance) 
{

  VectorF lDesiredVelocity =  AIMath::Vector2D(getOwner()->getPosition() - lTargetPos);

  if (lDesiredVelocity.isZero()) { //exact same position ?! bad!
	  lDesiredVelocity =  VectorF(0.0001f,0.0001f,0);
  }

  if (lDesiredVelocity.len() > lPanicDistance ) {
	  lDesiredVelocity.zero();
	  return (lDesiredVelocity);
  }
  lDesiredVelocity.normalize();
  lDesiredVelocity *= getOwner()->getMaxForwardVelocity();
  return (lDesiredVelocity - getOwner()->getVelocity());
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getArriveForce(VectorF lTargetPos, SteeringDeceleration lDeceleration)
{
	

  VectorF lDesiredVelocity = AIMath::Vector2D(lTargetPos - getOwner()->getPosition());
  F32 lDist =  lDesiredVelocity.len();

  if (lDist > mMoveTolerance) {
	const F32 lDecelerationTweaker = 0.3f;

	// calc speed
	F32 lSpeed = lDist / ((F32) lDeceleration *  lDecelerationTweaker);
	lSpeed = mClampF(lSpeed,0,getOwner()->getMaxForwardVelocity());

	lDesiredVelocity = lDesiredVelocity * lSpeed / lDist;

	// when mMoveTolerance is too low it may cause some problems here ... getVelocity is crazy ? 
	
	return (lDesiredVelocity - getOwner()->getVelocity());
  }
  
  return VectorF(0.f,0.f,0.f);
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getPursuitVector(GameBase* lEvader, F32 lStopDistance) //bullshit, bool useStopDistanceForCloseTo)
{

	VectorF lToEvader = AIMath::Vector2D(lEvader->getPosition() - getOwner()->getPosition());


    //pre 2023-02-28 if ((mMoveTolerance+lStopDistance) > lToEvader.len()) {
   if ((mMoveTolerance + lStopDistance) > lToEvader.len() && AIMath::CheckObjectsLos(getOwner(), lEvader)) {
	    return (VectorF::Zero);
    }

	VectorF lThisHeading = getHeading2D(getOwner());
	F32 lRelativHeading = mDot(lThisHeading,getHeading2D(lEvader));
	F32 curSpeed = AIMath::Vector2D(getOwner()->getVelocity()).len();

	if (mDot(lToEvader,lThisHeading) > 0.f && (curSpeed ==0.f || mFabs(lRelativHeading) > 0.95f) /*~18°*/ ) 
	{
		return lEvader->getPosition();
	}


	VectorF lEvaderVelo = AIMath::Vector2D(lEvader->getVelocity());
	F32 lEvaderSpeed = lEvaderVelo.len(); 

	if (curSpeed+lEvaderSpeed==0.f)
		return lEvader->getPosition();

	F32 lLookAheadTime = lToEvader.len() / (curSpeed + lEvaderSpeed);

    
   //2023-04-01 April April :P
   /*
   if (useStopDistanceForCloseTo)
   {
      return AIMath::getPositionCloseToDestination(
         getOwner()->getPosition()
         , lEvader->getPosition() + lEvaderVelo * lLookAheadTime
         , lStopDistance
      );
   }
   */

   return lEvader->getPosition() + lEvaderVelo * lLookAheadTime;

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getPursuitForce( GameBase *lEvader, F32 lStopDistance) 
{
	
	VectorF lResultVec =  getPursuitVector(lEvader, lStopDistance);
	if (lResultVec.len() == 0.f) 
		return VectorF(0.f,0.f,0.f);
    return getSeekForce(lResultVec);

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getEvadeVector( GameBase *lPursuer , F32 lPanicDistance)
{
	VectorF lToPursuer = AIMath::Vector2D(lPursuer->getPosition() - getOwner()->getPosition());

	VectorF lPursuerVelo = AIMath::Vector2D(lPursuer->getVelocity());
	F32 lPursuerSpeed = lPursuerVelo.len(); 
	const F32 lcTimeEnhance = 1.3f;
	F32 curSpeed = AIMath::Vector2D(getOwner()->getVelocity()).len();
	if (curSpeed+lPursuerSpeed==0.f)
		curSpeed = 0.000001f;
	F32 lLookAheadTime = lToPursuer.len()*lcTimeEnhance / (curSpeed + lPursuerSpeed); 

	return lPursuer->getPosition() + lPursuerVelo * lLookAheadTime;

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getEvadeForce( GameBase *lPursuer, F32 lPanicDistance) 
{
	VectorF lResultVec =  getEvadeVector(lPursuer, lPanicDistance);
	if (lResultVec.len() == 0.f) 
		return VectorF(0.f,0.f,0.f);
	
	return getFleeForce(lResultVec, lPanicDistance);

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getWanderForce( F32 lRad, F32 lDist, F32 lJitter) 
{
  if (mWanderTarget.isZero())  { //init
	F32 theta = gRandGen.randF() * M_2PI;

	//create a vector to a target position on the wander circle
	 mWanderTarget = VectorF(lRad * mCos(theta),
                             lRad * mSin(theta),
							 0);

  }

    //first, add a small random vector to the target's position
  mWanderTarget += VectorF(AIMath::RandomClamped() * lJitter,AIMath::RandomClamped() * lJitter, 0);

  //reproject this new vector back on to a unit circle
  mWanderTarget.normalize();

  //increase the length of the vector to the same as the radius
  //of the wander circle
  mWanderTarget *= lRad;

  //move the target into a position WanderDist in front of the agent
  VectorF lTargetLocal = AIMath::Vector2D(mWanderTarget + VectorF(0, lDist, 0));
  

  //project the target into world space
  VectorF lWorldTarget = getPointToWorldSpace(lTargetLocal, getOwner()->getTransform());

  //and steer towards it
  VectorF lDesiredVelocity = AIMath::Vector2D(lWorldTarget - getOwner()->getPosition());
  lDesiredVelocity.normalize();;
  
  lDesiredVelocity *= getOwner()->getMaxForwardVelocity();

  return (lDesiredVelocity - getOwner()->getVelocity());

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getHidingPosition( VectorF lPosOb, F32 lRadiusOb, VectorF lPosTarget)
{
	
	const F32 lDistanceFromBoundary = 0.5f;  //Distance from Obstancle ?!
	F32 lDistAway = lRadiusOb + lDistanceFromBoundary;
	VectorF lToOb = lPosOb - lPosTarget;
	lToOb.normalize();

	return (lToOb * lDistAway) + lPosOb;
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getHideForce(GameBase *lTarget, SimGroup *lObstGroup, F32 mMinDistance, F32 mMaxDistance)
{

	VectorF lBestHidingSpot(0.f,0.f,0.f);
	if (AIMath::Distance2D(getOwner()->getPosition(),lTarget->getPosition()) > mMaxDistance) {
		return lBestHidingSpot;
	}
	bool lFound = false;
	F32 lDistToClosest = 999999.f;
//GameBase* lHidingObj = NULL;
	
	for (SimGroup::iterator itr = lObstGroup->begin(); itr != lObstGroup->end(); itr++)
	{
		GameBase* lCurOb = static_cast<GameBase*>(*itr);
		if (AIMath::Distance2D(lCurOb->getPosition(),lTarget->getPosition()) < mMinDistance) {
			continue;
		}

		F32 lCurRad = mSqrt(lCurOb->getWorldBox().len_x()*lCurOb->getWorldBox().len_y());
		VectorF lHidingSpot = getHidingPosition( lCurOb->getPosition(), lCurRad, lTarget->getPosition());
		F32 lDist = AIMath::Distance2D(lHidingSpot,getOwner()->getPosition());
		if ( lDist < lDistToClosest) {
			lDistToClosest=lDist;
			lBestHidingSpot=lHidingSpot;
			lFound = true;
//			lHidingObj = lCurOb;
		}

	}


	if (!lFound) {
		return getEvadeForce(lTarget);
	}

	
//Con::executef( mOwner, 6, "onDebugRender", Con::getFloatArg(lHidingObj->getPosition().x), Con::getFloatArg(lHidingObj->getPosition().y), Con::getFloatArg(lHidingObj->getPosition().z+4));
	return getArriveForce(lBestHidingSpot,fast);
}
// --------------------------------------------------------------------------------------------
/**
 * AISteering::getWallForce
 * 1.97.3 Only use 1 feeler to detect walls and onstancles.
 * feeler len is set in steering creation
 * Feeler 0=front
 * @return force

  PROBLEM: AI runs to the end of the wall then turn back! thats sucks!
     mWallSkipCounter / mWallSavedForce


  XXTH 2023 this really sucks!!!!!!!!!!!!!!!!!!

 */
// --------------------------------------------------------------------------------------------

VectorF AISteering::getWallForce( bool lStuck ) 
{

  //init:
  VectorF lSteeringForce(0.f,0.f,0.f);
//  F32 lDistToClosestIP = 999999.f;

  //feelers
  Point3F lLoc = getOwner()->getBoxCenter();
  RayInfo lRay;
  VectorF lPos, lPosLeft, lPosRight;
  F32 lHalfBoxWidth =  getOwner()->getWorldBox().len_x() / 2 + 0.25f;

  
  
  VectorF lHeading = getHeading2D(getOwner());

  //forward
  lPos = lHeading * mFeelerLen + lLoc;
  lPosLeft  = lPos;

  lPosLeft  = lPos;  lPosLeft.x -= lHalfBoxWidth;
  lPosRight = lPos;  lPosLeft.x += lHalfBoxWidth;

  

#ifdef TORQUE_DEBUG
  char nbuf1[64];
  char nbuf2[64];
  dSprintf(nbuf1,64, "%f %f %f",lLoc.x,lLoc.y,lLoc.z);
  dSprintf(nbuf2,64, "%f %f %f",lPos.x,lPos.y,lPos.z);
  getOwner()->setDataField(StringTable->insert("dLoc"),NULL,nbuf1);
  getOwner()->setDataField(StringTable->insert("dPos"),NULL,nbuf2);
#endif

  getOwner()->disableCollision();
   if (getOwner()->getContainer()->castRay( lLoc, lPos, mWallAvoidMask, &lRay)) 
   {
	   F32 dot = mDot(Point3F(0.0f,0.0f,1.0f), lRay.normal);//mFabs();
	   if (dot<0.55f)
	   {
		   VectorF lOverShoot = lPos - lRay.point;
		   lSteeringForce = (lHeading + lRay.normal) * lOverShoot.len();
	   }
   } else if (getOwner()->getContainer()->castRay( lLoc, lPosLeft, mWallAvoidMask, &lRay)) 
   {
	   F32 dot = mDot(Point3F(0.0f,0.0f,1.0f), lRay.normal);//mFabs();
	   if (dot<0.55f)
	   {
		   VectorF lOverShoot = lPos - lRay.point;
		   lSteeringForce = (lHeading + lRay.normal) * lOverShoot.len();
	   }
   } else    if (getOwner()->getContainer()->castRay( lLoc, lPosRight, mWallAvoidMask, &lRay)) 
   {
	   F32 dot = mDot(Point3F(0.0f,0.0f,1.0f), lRay.normal);//mFabs();
	   if (dot<0.55f)
	   {
		   VectorF lOverShoot = lPos - lRay.point;
		   lSteeringForce = (lHeading + lRay.normal) * lOverShoot.len();
	   }
   }

   
  getOwner()->enableCollision();


  return (lSteeringForce );
}


/* orig code prior 1.97.3
VectorF AISteering::getWallForce( bool lStuck ) 
{
  //init:
  VectorF lSteeringForce(0.f,0.f,0.f);
  F32 lDistToClosestIP = 999999.f;

  //feelers
  Point3F lLoc = getOwner()->getBoxCenter();



  U32 lMaxRays = 3; //1 || 3! 
  if (!mThreeWallRays) //1.97.3
		lMaxRays = 1;	

  RayInfo lRay[3];
  VectorF lPos[3];
  


  
  // adjust len on velocity
   F32 curSpeed = AIMath::Vector2D(getOwner()->getVelocity()).len();
   F32 lFleelerLen = getOwner()->getWorldBox().len_y() + curSpeed * 0.25f;


  //forward
  lPos[0] = getHeading(mOwner) * lFleelerLen + lLoc;
  if (lMaxRays > 1) {
	 //left
	lPos[1] = AIMath::RotateAroundOrigin(getHeading(mOwner), M_PI * 1.75f) * lFleelerLen * 0.5f + lLoc;
    //right
    lPos[2] = AIMath::RotateAroundOrigin(getHeading(mOwner), M_PI * 0.25) * lFleelerLen * 0.5f + lLoc;
  }

  S32 lTouchcount=0;
  S32 lClosestRay = -1;
  getOwner()->disableCollision();
  for (S32 i = 0; i < lMaxRays; i++)  {
   if (getOwner()->getContainer()->castRay( lLoc, lPos[i], mWallAvoidMask, &lRay[i])) 
   {
	   if ( true ) 
	   {
		   F32 dot = mDot(Point3F(0.0f,0.0f,1.0f), lRay[i].normal);//mFabs();
		   if (dot<0.55f)
		   {
			
			   lTouchcount++;
			   F32 lRayDist = (lRay[i].point - lLoc).len();
			   if (lRayDist < lDistToClosestIP) 
		       {
				   lDistToClosestIP = lRayDist;
			       lClosestRay = i;
			   }
		   }
	   }
	  
   }
   //Con::executef( mOwner, 6, "onDebugRender", Con::getFloatArg(lPos[i].x), Con::getFloatArg(lPos[i].y), Con::getFloatArg(lPos[i].z));
  }
  getOwner()->enableCollision();
  //darn it found a corner and play ping pong, maybe forever, i'll try a flee from forward feeler 
  if (lTouchcount == 3) {
	lSteeringForce = getFleeForce(lPos[0]);
  } else if (lClosestRay >= 0 ) {
	  VectorF lOverShoot = lPos[lClosestRay] - lRay[lClosestRay].point;
	  //Con::executef( mOwner, 6, "onDebugRender", Con::getFloatArg(lRay[lClosestRay].point.x), Con::getFloatArg(lRay[lClosestRay].point.y), Con::getFloatArg(lRay[lClosestRay].point.z));

	  // lSteeringForce = (lRay[lClosestRay].normal * lOverShoot.len());

	  if (lStuck && lClosestRay != 0)  {
		  VectorF lHeading = getHeading2D(getOwner());
		  //lHeading.normalize();
		  lSteeringForce = (lHeading + lRay[lClosestRay].normal) * lOverShoot.len();
	  } else {
		  lSteeringForce = (lRay[lClosestRay].normal * lOverShoot.len());
	  }
	  
  }



  return (lSteeringForce );

}
*/
// --------------------------------------------------------------------------------------------
VectorF AISteering::getInterPoseForce( GameBase *lAgentA, GameBase *lAgentB)
{
	VectorF lMidPoint = (lAgentA->getPosition() - lAgentB->getPosition()) / 2.f;
	F32 lTimeToReach = AIMath::Distance2D(getOwner()->getPosition(),lMidPoint) 
						/ getOwner()->getMaxForwardVelocity();

	VectorF lAPos = lAgentA->getPosition() + lAgentA->getVelocity() * lTimeToReach;
	VectorF lBPos = lAgentB->getPosition() + lAgentB->getVelocity() * lTimeToReach;
	lMidPoint = (lAPos + lBPos) / 2.f;

	return (getArriveForce(lMidPoint, fast));
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getPathForce(   )
{
	VectorF lSteeringForce(0.f,0.f,0.f);
	if (!mPath || !mPath->initDone() || mPath->isStopped() || !mPath->haveWayPoint() || mPath->inPause()) 
		return lSteeringForce;

	AiPathWayPoint *curPoint = mPath->getCurrentWayPoint();
	if (!curPoint)
		return lSteeringForce;

	if (AIMath::Distance2D(getOwner()->getPosition(),curPoint->pos) <= mMoveTolerance /*2023-02-28 sucks + lStopDistance*/) {
      if (curPoint->msToNext > 0 && curPoint->tag == 0) {
	      mPath->setCurrentPause();
	      curPoint->tag = 1;
	      // Con::executef(getOwner(), 3, "onPathPointReached", Con::getIntArg(getOwner()->getId()), Con::getIntArg(curPoint->checkPointId));
	      Con::executef(getOwner()->getDataBlock(), "onPathPointReached", Con::getIntArg(getOwner()->getId()), Con::getIntArg(curPoint->checkPointId));
	      return lSteeringForce;
      } else if (curPoint->tag == 1){
	      curPoint->tag = 0;
	      Con::executef(getOwner()->getDataBlock(), "onPathPointContinue", Con::getIntArg(getOwner()->getId()), Con::getIntArg(curPoint->checkPointId));
      }

      if (mPath->finished() || !mPath->setNextWayPoint())
      {
         mPath->Stop();

#ifdef AI_NAVIGATION
         if (mNavigationUnfinished) {
            planNavigationPath(mLastPathDestination);
            return lSteeringForce;
         }
#endif //AI_NAVIGATION

            Con::executef(getOwner()->getDataBlock(), "onPathFinished", Con::getIntArg(getOwner()->getId()), Con::getIntArg(curPoint->checkPointId));
            return lSteeringForce;
		}

		curPoint = mPath->getCurrentWayPoint();
	}

	if (!curPoint)
		return lSteeringForce;

	if (mPath->finished() || curPoint->msToNext > 0)
		lSteeringForce = getArriveForce(curPoint->pos, normal);
	else 
		lSteeringForce = getSeekForce(curPoint->pos);

	return lSteeringForce;
	

}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getOffsetPursuitForce(GameBase *lLeader,VectorF lOffset)
{

	VectorF lSteeringForce(0.f,0.f,0.f);

	VectorF lWorldOffsetPos = getPointToWorldSpace(lOffset, lLeader->getTransform());
	VectorF lToOffset = lWorldOffsetPos - getOwner()->getPosition();
	F32 lLookAheadTime =  lToOffset.len() / (getOwner()->getMaxForwardVelocity() + lLeader->getVelocity().len());

	//XXTH 120824 looks stuid when running arround an object which only move on the same point!
	// 150423 again... now a bit better :D
	if (lLeader->getVelocity().len() < 0.001f && lToOffset.len() < lOffset.len()*2.f )
	{
		// Con::printf("ToOffset=%f lOffset=%f lWorldOffsetPos=%f",lToOffset.len(),lOffset.len(),lWorldOffsetPos.len());
		return lSteeringForce;
	}

	lSteeringForce = getArriveForce(lWorldOffsetPos + lLeader->getVelocity() * lLookAheadTime, fast);
	//lSteeringForce = getSeekForce(lWorldOffsetPos + lLeader->getVelocity() * lLookAheadTime);
	return lSteeringForce;
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getSeparationForce(F32 lRadius, const SimpleQueryList	*lNeighbours, S32 lIgnorePlayerType)
{
	VectorF lSteeringForce(0.f,0.f,0.f);
	if (lNeighbours->mList.size() == 0) 
		return lSteeringForce;


   //SceneObject* curAgent; << crashed if invalid!
// pre 2023-03-08     SimObjectPtr<SceneObject> curAgent;
   SimObjectPtr<ShapeBase> curAgent;
   SimObjectPtr<ShapeBase> myOwner = dynamic_cast<ShapeBase*>(getOwner());
      for (S32 a = 0; a < lNeighbours->mList.size(); ++a)
      {
         curAgent = dynamic_cast<ShapeBase*>(lNeighbours->mList[a]);
         if (curAgent.isNull())
            continue;


/* old Auteria:
         if (lIgnorePlayerType >= 0 && curAgent->getPlayerType() == lIgnorePlayerType)
            continue;
*/

         if (curAgent->getFaction() != myOwner->getFaction())
            continue;


		VectorF lToAgent = AIMath::Vector2D(getOwner()->getPosition() - curAgent->getPosition());
		 if (lToAgent.isZero()) { //exact same position ?! bad!
			lToAgent =  VectorF(0.0001f,0.0001f,0.f);
		 }
		F32 lToLen = lToAgent.len();
		if (lToLen > lRadius) 
			continue;
		if (lToLen == 0.f) //devide zero is bad
			lToLen = 0.00000001f;
		lToAgent.normalize();
		lSteeringForce += lToAgent / lToLen;
	}
	return lSteeringForce;
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getAlignmentForce(F32 lRadius, const SimpleQueryList	*lNeighbours)
{
	VectorF lSteeringForce(0.f,0.f,0.f);
	if (lNeighbours->mList.size() == 0) 
		return lSteeringForce;
	S32 lCnt = 0;
	// SceneObject* curAgent; << can crash!
    SimObjectPtr<SceneObject> curAgent;
   for (S32 a = 0; a<lNeighbours->mList.size(); ++a)
   {
		curAgent = lNeighbours->mList[a];
        if (curAgent.isNull())
           continue;

		VectorF lToAgent = AIMath::Vector2D(getOwner()->getPosition() - curAgent->getPosition());
		if (lToAgent.len() > lRadius) 
			continue;

		lSteeringForce += getHeading2D(static_cast<GameBase*>(curAgent.getObject()));
		++lCnt;
	}
	if ( lCnt > 0) {
		lSteeringForce /= (F32)lCnt;
		lSteeringForce -= getHeading2D(getOwner());
	}

	return lSteeringForce;
}
// --------------------------------------------------------------------------------------------
VectorF AISteering::getCohesionForce(F32 lRadius, const SimpleQueryList	*lNeighbours)
{
	VectorF lSteeringForce(0.f,0.f,0.f);
	VectorF lCenterofMass(0.f,0.f,0.f);
	if (lNeighbours->mList.size() == 0) 
		return lSteeringForce;

	S32 lCnt = 0;
   // SceneObject* curAgent; << can crash!
   SimObjectPtr<SceneObject> curAgent;
   for (S32 a = 0; a<lNeighbours->mList.size(); ++a) {
		curAgent = lNeighbours->mList[a];
        if (curAgent.isNull())
              continue;

		VectorF lToAgent = AIMath::Vector2D(getOwner()->getPosition() - curAgent->getPosition());
		if (lToAgent.len() > lRadius) 
			continue;

		lCenterofMass += curAgent->getPosition();
		++lCnt;
	}
	if ( lCnt > 0) {
		lCenterofMass /= (F32)lCnt;
		lSteeringForce = getSeekForce(lCenterofMass);
	}


	return lSteeringForce;
}
// --------------------------------------------------------------------------------------------
// PATH
// --------------------------------------------------------------------------------------------
// Usual Path Object with possible spline movement!

bool AISteering::clearPathObject()
{
      mPath->clearPoints();
      return true;
}

bool AISteering::setPathObject( SimPath::Path *pathObject, bool lLooping, bool reverse)
{
	if (!pathObject) {
		mPath->clearPoints();
		return true;
	}

	mPath->clearPoints();
	mPath->setLooping(lLooping);

   bool SplineAllowed = pathObject->size() > 2; //and looping ?! 


    // IF REVERSE SPLINE IS NOT IMPLEMENTED!!!
	if (reverse) {
		for (S32 i = pathObject->size()-1; i >= 0; i--)
		{
			Marker* pMarker = static_cast<Marker*>((*pathObject)[i]);
			if (pMarker != NULL) {
				AiPathWayPoint lNewPoint;
				lNewPoint.pos = pMarker->getPosition();
				lNewPoint.msToNext = pMarker->mMSToNext;
				lNewPoint.trigger  = false;
				lNewPoint.checkPointId = pMarker->mSeqNum;
				mPath->addPoint(lNewPoint);

				if (SplineAllowed && mUseSplinePath && pMarker->mSmoothingType == Marker::SmoothingTypeSpline) {
                     for (F32 t = mPathSplinePrec; t < 1; t += mPathSplinePrec) {
                        mPath->addPoint(AIMath::getSubNodePositionReverse(pathObject, i, t));
                     }
				} 
			}
		} //for
	} else {
		for (S32 i = 0; i < pathObject->size(); i++)
		{
			Marker* pMarker = static_cast<Marker*>((*pathObject)[i]);
			if (pMarker != NULL) {
				AiPathWayPoint lNewPoint;
				lNewPoint.pos = pMarker->getPosition();
				lNewPoint.msToNext = pMarker->mMSToNext;
				lNewPoint.trigger  = false;
				lNewPoint.checkPointId = pMarker->mSeqNum;
				mPath->addPoint(lNewPoint);

				if (SplineAllowed && mUseSplinePath && pMarker->mSmoothingType == Marker::SmoothingTypeSpline) {
					for (F32 t = mPathSplinePrec ; t < 1 ; t+=mPathSplinePrec) {
						mPath->addPoint(AIMath::getSubNodePosition(pathObject,i,t));
					}
				}
			}
		} //for
	}

    mPath->init(getOwner()->getPosition());

	return true;
}
// --------------------------------------------------------------------------------------------
// instead of a SimPath we got a iAIPath to follow ;)
bool AISteering::setIAIPath(iAIPath* pathObject)
{
   if (!pathObject)
      return false;

   mPath->clearPoints();
   mPath->setLooping(false);

   // mPath->addPoint(getOwner()->getPosition());

   for (S32 i = 0; i < pathObject->nodeCount(); i++) {
      mPath->addPoint(pathObject->getNodePosition(i));
   }
   mPath->init(VectorF(0.f, 0.f, 0.f), true);


   return true;
}
// --------------------------------------------------------------------------------------------
/**
 * untested used when under terrain
 */
bool AISteering::resetPath() {
	if (hasPath())
	{
			mPath->init(getOwner()->getPosition());
			return true;
	} 
	return false;
}
// --------------------------------------------------------------------------------------------
bool AISteering::stopPath() {
	mPath->Stop();
	return true;
}
bool AISteering::continuePath() {
	mPath->Resume();
	return true;
}

Point3F* AISteering::getClosestWayPoint(Point3F lFrom) {
	AiPathWayPoint* wp;
	wp = mPath->getClosestWayPoint(lFrom);
	if (wp) {
		return &wp->pos;
	}
	
  return NULL;
}
// --------------------------------------------------------------------------------------------
#ifdef AI_USEPATH

#ifdef AI_NAVIGATION
//THIS works great and fix the unfinished path shit seamless
bool AISteering::planNavigationPath(Point3F lTo)

{
   mPath->clearPoints();

   NavPath* lPath;
   bool lPathIsRegistered = false;
   if (!mPtrNavPath.isNull())
   {
      //  mPtrNavPath->deleteObject();
      lPath = mPtrNavPath;
      lPathIsRegistered = true;
   }
   else {
      lPath = new NavPath();
   }

   lPath->mMesh = getNavMesh();
   lPath->mFrom = getOwner()->getPosition();
   lPath->mTo = lTo;
   lPath->mFromSet = lPath->mToSet = true;
   lPath->mAlwaysRender = true;
   lPath->mLinkTypes = AllFlags; //FIXME variable ?! 
   lPath->mXray = true;
   //lPath->mIsSliced = true; //mmmm does also not helps path is not complete!
   // Paths plan automatically upon being registered.

   if (!lPathIsRegistered && !lPath->registerObject())
   {
      delete lPath;
      return false;
   }
   else {
      lPath->plan();
      mPtrNavPath = lPath;

   }

   if (mPtrNavPath->success() && mPtrNavPath->size() > 0) {
      //check we need to replan !!
      mNavigationUnfinished = false;
      if (AIMath::Distance2D(lTo, mPtrNavPath->getNode(mPtrNavPath->size() - 1)) > mMoveTolerance) {
         mNavigationUnfinished = true;
      }

      S32 lStartNodeIdx = (mPtrNavPath->size() > 1) ? 1 : 0;
      

      //no need! mPath->addPoint(getOwner()->getPosition());
      for (S32 i = lStartNodeIdx; i < mPtrNavPath->size(); i++)
      {
         mPath->addPoint(mPtrNavPath->getNode(i));
      }
      //no need! mPath->addPoint(lTo); 
      mPath->init(VectorF(0.f, 0.f, 0.f), true);

      return true;
   }



   return false;
}
#endif //AI_NAVIGATION


// --------------------------------------------------------------------------------------------
bool AISteering::setPathDestination( Point3F lTo, bool lForce)
{

   F32 lDiffDist = mFabs((mLastPathDestination - lTo).len());
   if (!lForce) // force a new path
   {
      //mMoveTolerance is not enough here !!
      if (mLastPathDestination == lTo || lDiffDist <= mMoveTolerance)
         return true;
   }

	mLastPathDestination = lTo;
	//bad idea since i use mLastPathStart : mPath->clearPoints();
	mPath->setLooping(false);

	
   if (!mPathMapInitDone)
	   initPathMap();

    bool lFound = false;
#ifdef AI_NAVIGATION
    if (!mNavMesh
#else
	if (!mPathMap
#endif
		 || mPath->checkLos(getOwner()->getPosition(), lTo)
		 //also SUX || !mPathMap->validPos(getOwner()->getPosition()) 
		 //SUX! || !mPathMap->validPos(lTo) 
		 
		 ) 
	{
		mPath->clearPoints();
		mPath->addPoint(lTo);
		mPath->init(getOwner()->getPosition());
		lFound = true;
	} 
	else
#ifdef AI_NAVIGATION
   


/*
THIS DOES NOT WORK!!!The PATH is OFTEN unfinished at the some point but says it's finished grrrrr

i dont want to hack in detour so i must hack here like they did in aiPlayer where it's replan the path
from position 

mhh ok we have the 
   mPtrNavPath
and onPathFinishied is called ... this is where I need to restart the path
alter mFrom and plan again ?,or replace the pointer to the path if changing from not works ...
mhh why do we need to add this shit every call ? it can stay until done
   
==> moved to planNavigationPath

*/


   if (planNavigationPath(lTo))
   {

      lFound = true;
   }
   else {
      Con::executef(getOwner()->getDataBlock(), "onPathFailed", Con::getIntArg(getOwner()->getId()));
   }
   



#else  // AI_NAVIGATION iAIPath => 
	 if (mPathMap && !mPathMap->validPos(getOwner()->getPosition()))
	{
		//we better do noting here, when we have no los! ! 
#ifdef TORQUE_DEBUG
		Con::printf("We have a pathmap but no valid grid under the feet! obj:%s",getOwner()->getIdString());
#endif
	}
	else  
	{

		if (mPath->haveWayPoint() && !mPath->isStopped() &&  lDiffDist < 5.f )
		{
			iAIPathNode* lFirstNode = mPathMap->getClosestNode(getOwner()->getPosition());
			if (  lFirstNode &&
				  (
			           (mPath->getLastWayPoint() && mPath->getLastWayPoint()->pos == lFirstNode->mPosition)
			   	    || (mPath->getCurrentWayPoint() && mPath->getCurrentWayPoint()->pos == lFirstNode->mPosition )
				  )
				)
				return true;
		}


		mPath->clearPoints();
	    iAIPath*  lPath;	
  	    lPath = new iAIPath();
		if (lPath->createPath(mPathMap, getOwner()->getPosition(), lTo, false)) 
		{ 
			mPath->addPoint(getOwner()->getPosition());
			while (lPath->hasNextNode()) {
				mPath->addPoint(lPath->getNextPosition());
			}
			mPath->addPoint(lTo);
			mPath->init(VectorF(0.f,0.f,0.f), true);

			//MMMHHH smooth suck inside interior!, also if it looks strange sometime smooth off here!
			//but looks much better! but sucks :( try smooth on createpath!
			//:'( mPath->smooth();

			lFound = true;
		} 
		SAFE_DELETE(lPath);
	}
#endif
   return lFound;
}
#endif
// --------------------------------------------------------------------------------------------
#ifdef AI_USEPATH
bool AISteering::updatePursuitPath( GameBase *lEvader ) 
{
	VectorF lResultVec =  getPursuitVector(lEvader);
	if (lResultVec.len() == 0.f) 
		return false;

	return this->setPathDestination(lResultVec);
}
// --------------------------------------------------------------------------------------------
void AISteering::initPathMap() 
{
#ifdef AI_NAVIGATION
   if (mPathMapInitDone) 
	   return;
   mPathMapInitDone = true;
   mNavMesh = NULL;

   SimSet* set = NavMesh::getServerSet();
   //XXTH we support only one NavMesh at the moment fixme
   // look at aiEnhancedPlayer::findNavMesh but you need to find a better way than
   // allways search for the mesh 
   if (set->size() > 0)
   {
      if (set->size() > 1)
      {
         Con::errorf("aiSteering only support ONE NavMesh at the moment!!! ");
      }
      mNavMesh = static_cast<NavMesh*>(set->at(0));
   }

#else
   if (!Sim::findObject(dAtoi(Con::getVariable("$iAIPathMap")), mPathMap))
		mPathMap = NULL;
#endif  //AI_NAVIGATION
}
#endif
// --------------------------------------------------------------------------------------------
void AISteering::throwOwnerCallback( const char *name)
{
    //Con::executef(getOwner(), 2, name, Con::getIntArg(getOwner()->getId()));
	Con::executef(getOwner()->getDataBlock(),  name, Con::getIntArg(getOwner()->getId()));

}

