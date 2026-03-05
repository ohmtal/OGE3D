//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// // AIPath by T.Huehn 2009 (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------
#ifndef _AIPATH_H_
#define _AIPATH_H_


#ifndef _MPOINT_H_
#include "math/mPoint3.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class AiPathWayPoint
{
public:
	AiPathWayPoint() { pos=Point3F(0.f,0.f,0.f);msToNext=0;trigger =false; checkPointId = -1; tag = 0; inInterior = false; inWater = false; }
	AiPathWayPoint( const Point3F lPos) { pos=lPos; msToNext=0; trigger =false;checkPointId = -1; tag = 0; inInterior = false; inWater = false; }
    Point3F  pos;
    U32      msToNext;
	bool     trigger ;
	S32		 checkPointId;
	bool	 inInterior;
	bool	 inWater;
	S32		 tag;

};

class AIPath {
private:

	Vector<AiPathWayPoint>	mWayPoints;
	Vector<AiPathWayPoint>::iterator  mCurWayPoint;
    AiPathWayPoint*  mLastWayPoint;
	bool mIsLooping;
	bool mStop;
	bool mInitDone;
	bool setWayPoint(AiPathWayPoint* newPoint);
	U32  mPausedUntil;
   bool mDebug;
   bool mSplinized;

public:
	AIPath();
	~AIPath();
	
	U32 mColMask;
	void init(Point3F fromPoint, bool lUseFirstPoint = false);
	bool initDone();
	bool haveNodes() { return mWayPoints.size() > 0; }
    AiPathWayPoint* getClosestWayPoint(Point3F lFrom);
	bool finished() { return (!mIsLooping && mCurWayPoint == mWayPoints.end()); }
	bool inProcess() { return (haveNodes() && ! finished()); }
	void addPoint( const AiPathWayPoint &location );
   void splineize(F32 lSplinePrec = 0.1f);
   bool setNextWayPoint();
    AiPathWayPoint* getCurrentWayPoint();
	AiPathWayPoint* getLastWayPoint();
	bool haveWayPoint() { return mInitDone && mCurWayPoint != NULL;}
   void clearPoints() { mWayPoints.clear(); mCurWayPoint = NULL; mLastWayPoint = NULL; mSplinized = false; }

	void Stop() { mStop = true; }
	void Resume() { 
					mStop = false; 
					if (inPause())
					{
						mPausedUntil = 0;
					}

	              }
	bool isStopped() { return mStop; }

	void setLooping(bool lValue) { mIsLooping = lValue; }
	bool getLooping() { return mIsLooping; }

	bool checkLos(Point3F from, Point3F to, U32 lMask = 0);
	void smooth();

	void setCurrentPause(); 
	bool inPause();

   void setDebug(bool lValue) { mDebug = lValue; }
};




#endif // _AIPATH_H_
