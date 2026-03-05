//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//  AIPath by T.Huehn 2009 (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------

#include "aiPath.h"
#include "aiMath.h"

//--------------------------------------------------------------------------
AIPath::AIPath()
{
	mIsLooping=true;
	mStop = false;
	mPausedUntil = 0;
	mCurWayPoint = NULL;
	mLastWayPoint = NULL;
   mColMask = StaticShapeObjectType  |  TerrainObjectType | TerrainLikeObjectType | InteriorLikeObjectType;
//TGE:  mColMask = InteriorObjectType | StaticShapeObjectType  | StaticTSObjectType | TerrainObjectType;
   mDebug = false;
   mSplinized = false;
}

AIPath::~AIPath()
{
	mWayPoints.clear();
}
//--------------------------------------------------------------------------
void AIPath::init(Point3F fromPoint, bool lUseFirstPoint) {

	if (mWayPoints.size() == 0)
		return;

	mLastWayPoint = NULL;
	if (lUseFirstPoint) {
	   mCurWayPoint = mWayPoints.begin();
	} else {
		setWayPoint(getClosestWayPoint(fromPoint));
	}
	mStop = false;
	mInitDone=true; 
}

bool AIPath::initDone() { 
	if (mInitDone && mWayPoints.size() == 0) {
		mInitDone=false;
	}
	return mInitDone; 
}
//--------------------------------------------------------------------------
void AIPath::addPoint( const AiPathWayPoint &location)
{
	
	mWayPoints.push_back(location);
#ifdef TORQUE_DEBUG
   if (mDebug)
   {
      Con::printf("AIPath::addPoint:%d = %f,%f,%f", mWayPoints.size(), location.pos.x, location.pos.y, location.pos.z);
      Con::executef("GlobalOnDebugRender", Con::getFloatArg(location.pos.x), Con::getFloatArg(location.pos.y), Con::getFloatArg(location.pos.z));
   }
#endif
}
//--------------------------------------------------------------------------
void AIPath::splineize(F32 lSplinePrec)
{
   if (mSplinized) //really again ? 
      return;

   if (mWayPoints.size() < 3)
      return;
   //save the current Points
   Vector<AiPathWayPoint>	lWayPoints = mWayPoints;
   clearPoints(); //mSplinized is set to false which is ok 
   AiPathWayPoint lNewPoint;
   for (S32 i = 0; i < lWayPoints.size(); i++)
   {
      addPoint(lWayPoints[i]);
      for (F32 t = lSplinePrec; t < 1; t += lSplinePrec) {
         addPoint(AIMath::getSubNodePosition(lWayPoints, i, t));
      }
   }
   mSplinized = true;

#ifdef TORQUE_DEBUG
   Con::printf("AIPath splineized precision: %f, points before: %d points after: %d", lSplinePrec, lWayPoints.size(), mWayPoints.size());
#endif // TORQUE_DEBUG

}
//--------------------------------------------------------------------------
bool AIPath::setWayPoint(AiPathWayPoint* newPoint) {
    
	if (mWayPoints.size()<1) 
		return false;
	
	for (Vector<AiPathWayPoint>::iterator itr = mWayPoints.begin(); itr != mWayPoints.end(); itr++)
     {
        AiPathWayPoint* pWayPoint = itr;
		if (pWayPoint == newPoint) {
			mCurWayPoint=itr;
			return true;
		}
	 }
   
	return  false;
}
//--------------------------------------------------------------------------
bool AIPath::setNextWayPoint() {

	if (mWayPoints.size()<1) 
		return false;

	mLastWayPoint=mCurWayPoint;
	
	if (mCurWayPoint == mWayPoints.end() || ++mCurWayPoint == mWayPoints.end()) {
		if (mIsLooping) {
			mCurWayPoint = mWayPoints.begin();
		} else {
			return false; //no loop no more waypoints
		}
	}
	return true;
}
//--------------------------------------------------------------------------
AiPathWayPoint* AIPath::getCurrentWayPoint()
{
	if (!mCurWayPoint || !mInitDone) 
		return NULL;

    return ( mCurWayPoint );
}
//--------------------------------------------------------------------------
AiPathWayPoint* AIPath::getLastWayPoint()
{
	if (!mLastWayPoint || !mInitDone) 
		return NULL;

    return ( mLastWayPoint );
}
//--------------------------------------------------------------------------
AiPathWayPoint* AIPath::getClosestWayPoint(Point3F lFrom)
{
	AiPathWayPoint* result = 0;
	if (mWayPoints.size()<1) 
		return result;

	F32 DistToClosest = 999999.f;
	for (Vector<AiPathWayPoint>::iterator itr = mWayPoints.begin(); itr != mWayPoints.end(); itr++)
    {
		F32 Dist = AIMath::Distance2D(lFrom, itr->pos);
	  if (Dist < DistToClosest) {
		result = itr;
		DistToClosest = Dist;
	  }
	}
//	Con::printf("AIPath::getClosestWayPoint: %f,%f,%f",result->x,result->y,result->z);
	return result;
}

// --------------------------------------------------------------------------------------------
bool AIPath::checkLos(Point3F from, Point3F to, U32 lMask)
{
	
	if (lMask == 0) 
		lMask = mColMask;

	//math add 0.5 but i want 0.75 here ! 
	from.z+=0.25f;
	to.z+=0.25f;

	return AIMath::checkLos(from,to,lMask);
}
// --------------------------------------------------------------------------------------------
// quick edge smooth
void AIPath::smooth() {
	if (mWayPoints.size()<3) 
		return;

	bool erasedNode = false;
	U32 iter = 0;

	while (iter < (mWayPoints.size()-2))
	{
		erasedNode = false;

		if (this->checkLos(mWayPoints[iter].pos, mWayPoints[iter+2].pos))
		{
#ifdef TORQUE_DEBUG
			Con::printf("AIPath::smooth Waypoint removed: %f,%f,%f",mWayPoints[iter+1].pos.x,mWayPoints[iter+1].pos.y,mWayPoints[iter+1].pos.z );
#endif
			mWayPoints.erase(iter+1);
			erasedNode = true;
		}
		if (!erasedNode)
			iter++;
	} //while

}
// --------------------------------------------------------------------------------------------
void AIPath::setCurrentPause() {
	mPausedUntil = Sim::getCurrentTime() + mCurWayPoint->msToNext;
}
bool AIPath::inPause() {
	return mPausedUntil > Sim::getCurrentTime();
}

