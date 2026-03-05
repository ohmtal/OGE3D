//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// 
// AIMath by t.huehn (XXTH) (c) huehn-software 2009
// Note should have called it aiTools, since not all is math here!
//-----------------------------------------------------------------------------

#ifndef _AIMATH_H_
#define _AIMATH_H_

#ifndef _MPOINT_H_
#include "math/mPoint3.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MQUAT_H_
#include "math/mQuat.h"
#endif

#ifndef _SIMPATH_H_
#include "scene/simPath.h"
#endif

#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif

#ifndef _MISSIONAREA_H_
#include "T3D/missionArea.h"
#endif

#ifndef _EARLYOUTPOLYLIST_H_
#include "collision/earlyOutPolyList.h"
#endif

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _MATERIALDEFINITION_H_
#include "materials/materialDefinition.h"
#endif
#ifndef _BASEMATINSTANCE_H_
#include "materials/baseMatInstance.h"
#endif

#ifndef _BASEMATINSTANCE_H_
#include "materials/baseMatInstance.h"
#endif

#ifndef _AIPATH_H_
#include "aiPath.h"
#endif

//------------------------------------------------------------------------
// AIMath
//------------------------------------------------------------------------
namespace AIMath
{
inline S32 Sign2D(const Point3F &p1, const Point3F &p2)
{
	if (p1.y*p2.x > p1.x*p2.y) 
		return 1;  //clockwise
	else
		return -1; //anticlockwise
}

//------------------------------------------------------------------------
inline VectorF Vector2D(VectorF lVec ) {
	lVec.z=0;
	return lVec;
}

//------------------------------------------------------------------------
inline F32 Distance2D(Point3F v1, Point3F v2) {

   VectorF v = v2 - v1;
   return mSqrt((v.x * v.x) + (v.y * v.y));
}

inline F32 Distance3D(Point3F v1, Point3F v2) {
   VectorF v = v2 - v1;
   return mSqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

//------------------------------------------------------------------------
inline bool isVisible(Point3F objPos, Point3F camPos, VectorF camVec, F32 maxDist, F32 minDist = 0)
{
   //build the vector to the object
   VectorF objVec = objPos - camPos;
   // now check distance
   F32 lDist = mSqrt((objVec.x * objVec.x) + (objVec.y * objVec.y) + (objVec.z * objVec.z));
   if (lDist < minDist)
	   return true;
   if (lDist > maxDist)
	   return false;


   //objVec.normalize();
   F32 lDot = mDot(objVec,camVec);
   if (lDot <  0.f) //object is behind
	   return false;

   return true;

}
//------------------------------------------------------------------------
inline VectorF VectorPerp2D(VectorF lVec) //return Side!
{
  return VectorF(-lVec.y, lVec.x, 0);
}

// --------------------------------------------------------------------------------------------
// Note 360 == M_2PI_F
inline VectorF RotateAroundOrigin(VectorF v,F32 radians) {

   EulerF rot(0.0f,0.0f, radians);
   QuatF rotQ(rot);
   AngAxisF aa;
   aa.set(rotQ);

   MatrixF temp1(true);
   aa.setMatrix(&temp1);

   Point3F result;
   temp1.mulV(v, &result);

   return result;

}
//------------------------------------------------------------------------
//returns a random double in the range -1 < n < 1
inline F32 RandomClamped()    {
	return gRandGen.randF() - gRandGen.randF();
}
//------------------------------------------------------------------------
// SimPath tools for spline:
//------------------------------------------------------------------------
inline Point3F getPartnerNodePosition(SimPath::Path *path, S32 id, S32 offset) {

	S32 cnt = path->size();
	S32 res = id + offset;

    if (res < 0) {
     res = cnt + res; 
  } else {
     res = res % cnt; 
  }

  Marker* pMarker = static_cast<Marker*>((*path)[res]);

  return pMarker->getPosition();
}

// --------------------------------------------------------------------------------------------
inline Point3F getSubNodePosition(SimPath::Path *path, S32 id, F32 t)
{
   Point3F result;
   result.zero();

	//for catmul we need 4 points:
	Marker* pMarker = static_cast<Marker*>((*path)[id]);
	VectorF p0 = getPartnerNodePosition(path, id, -1);
	VectorF p1 = pMarker->getPosition();
	VectorF p2 = getPartnerNodePosition(path, id, 1);
	VectorF p3 = getPartnerNodePosition(path, id, 2);

	
	result.x = mCatmullrom(t, p0.x,p1.x,p2.x,p3.x);
	result.y = mCatmullrom(t, p0.y,p1.y,p2.y,p3.y);
	result.z = mCatmullrom(t, p0.z,p1.z,p2.z,p3.z);

	return result;
}
// --------------------------------------------------------------------------------------------
inline Point3F getSubNodePositionReverse(SimPath::Path* path, S32 id, F32 t)
{
   //for catmul we need 4 points:
   Marker* pMarker = static_cast<Marker*>((*path)[id]);
   VectorF p0 = getPartnerNodePosition(path, id, 1);
   VectorF p1 = pMarker->getPosition();
   VectorF p2 = getPartnerNodePosition(path, id, -1);
   VectorF p3 = getPartnerNodePosition(path, id, -2);

   Point3F result;
   result.x = mCatmullrom(t, p0.x, p1.x, p2.x, p3.x);
   result.y = mCatmullrom(t, p0.y, p1.y, p2.y, p3.y);
   result.z = mCatmullrom(t, p0.z, p1.z, p2.z, p3.z);

   return result;
}

// --------------------------------------------------------------------------------------------
// for aiPath
inline Point3F getPartnerNodePosition(Vector<AiPathWayPoint> lPoints, S32 id, S32 offset) {

   S32 cnt = lPoints.size();
   S32 res = id + offset;

   if (res < 0) {
      res = cnt + res;
   }
   else {
      res = res % cnt;
   }

   return lPoints[res].pos;
}

// --------------------------------------------------------------------------------------------
// for aiPath
inline Point3F getSubNodePosition(Vector<AiPathWayPoint> lPoints, S32 id, F32 t)
{
   Point3F result;
   result.zero();

   if (lPoints.size() < 3 || id > lPoints.size() - 1)
   {
      Con::errorf("aiMath::getSubNodePosition invalid id (%d)! or path points size (%d)", id, lPoints.size());
      return result;
   }


   //for catmul we need 4 points:
   VectorF p0 = getPartnerNodePosition(lPoints, id, -1);
   VectorF p1 = lPoints[id].pos;
   VectorF p2 = getPartnerNodePosition(lPoints, id, 1);
   VectorF p3 = getPartnerNodePosition(lPoints, id, 2);


   result.x = mCatmullrom(t, p0.x, p1.x, p2.x, p3.x);
   result.y = mCatmullrom(t, p0.y, p1.y, p2.y, p3.y);
   result.z = mCatmullrom(t, p0.z, p1.z, p2.z, p3.z);

   return result;
}
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
/**
 * get a terrain on position
 * 
 * Point3F pos                   3D pos where we want to look
 * bool isServerContainer  true  on server or client container
 * F32 searchOffSet = 1000.f     how many meters we look up and down 
 * 
*/
inline TerrainBlock* getTerrainBlockATpos(Point3F pos, bool isServerContainer = true, F32 searchOffSet = 1000.f)
{
   U32 conformMask = (TerrainObjectType );
   RayInfo surfaceInfo;

   Point3F above = Point3F(pos.x, pos.y, pos.z + searchOffSet);
   Point3F below = Point3F(pos.x, pos.y, pos.z - searchOffSet);

   bool found = false;
   if (isServerContainer) {
      if (gServerContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }
   else {
      if (gClientContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }

   if (found) {
      return static_cast<TerrainBlock*>(surfaceInfo.object);

   }
   return NULL;
}



inline Material* getTerrainMaterialAtpos(Point3F pos, bool isServerContainer = true, F32 searchOffSet = 1000.f)
{
   U32 conformMask = (TerrainObjectType);
   RayInfo surfaceInfo;

   Point3F above = Point3F(pos.x, pos.y, pos.z + searchOffSet);
   Point3F below = Point3F(pos.x, pos.y, pos.z - searchOffSet);

   bool found = false;
   if (isServerContainer) {
      if (gServerContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }
   else {
      if (gClientContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }


   if (found) {
      return (surfaceInfo.material ? dynamic_cast<Material*>(surfaceInfo.material->getMaterial()) : NULL);
   }
   return NULL;
}



/**
 * getTerrainHeight not really math but good place to use it!
 * T3D absolute heightpoint because terrain must not be located at z=0
*/
inline F32 getTerrainHeight(Point3F pos, TerrainBlock * lTerrain=NULL , bool onServer = true) {
   
   F32 height = 0.0f;
   TerrainBlock * terrain;
   if (lTerrain != NULL)
      terrain = lTerrain;
   else
      terrain = getTerrainBlockATpos(pos, onServer);

   if(terrain)
   {
         Point3F offset;
         terrain->getTransform().getColumn(3, &offset);
         pos -= offset;
         terrain->getHeight(Point2F(pos.x,pos.y), &height);
         F32 terrZ = terrain->getPosition().z;
         height += terrZ;
   }

   return height;
}



inline Point3F getTerrainNormal(Point3F pos, TerrainBlock * lTerrain=NULL, bool onServer=true ) {
   
   Point3F normal = Point3F(0.f,0.f,0.f);
   TerrainBlock * terrain;
   if (lTerrain != NULL)
      terrain = lTerrain;
   else
      terrain = getTerrainBlockATpos(pos, onServer); //dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
   if(terrain)
   {
         Point3F offset;
         terrain->getTransform().getColumn(3, &offset);
         pos -= offset;
         terrain->getNormal(Point2F(pos.x,pos.y), &normal, true);
   }
   return normal;
}


inline Point3F getTerrainPoint(Point3F pos, bool onServer = true) {
   
	pos.z=AIMath::getTerrainHeight(pos, NULL, onServer);
	return pos;
}



// --------------------------------------------------------------------------------------------
inline bool PointInMissionArea(Point3F pos) {


  MissionArea* obj = MissionArea::getServerObject();
  //TGE MissionArea * obj = dynamic_cast<MissionArea*>(Sim::findObject("MissionArea"));
  if(!obj)
      return true; //true by default!

	// Checks to see if the player is in the Mission Area...
   const RectI &area = obj->getArea();
   
   if ((pos.x < area.point.x || pos.x > area.point.x + area.extent.x ||
       pos.y < area.point.y || pos.y > area.point.y + area.extent.y)) {
		   return false;
   }
   return true;
//TGE was:  return obj->PointInArea(pos.x,pos.y);
}
// --------------------------------------------------------------------------------------------
inline bool _boxEmpty(Box3F B, U32 mask, bool onClient = false) 
{
   SceneContainer* lContainer;
   if (onClient)
	   lContainer = &gClientContainer;
   else
	   lContainer = &gServerContainer;

   EarlyOutPolyList polyList;
   polyList.mPlaneList.clear();
   polyList.mNormal.set(0,0,0);
   polyList.mPlaneList.setSize(6);
   bool result = false;
   polyList.mPlaneList[0].set(B.minExtents, VectorF(-1,0,0));
   polyList.mPlaneList[1].set(B.maxExtents, VectorF(0,1,0));
   polyList.mPlaneList[2].set(B.maxExtents, VectorF(1,0,0));
   polyList.mPlaneList[3].set(B.minExtents, VectorF(0,-1,0));
   polyList.mPlaneList[4].set(B.minExtents, VectorF(0,0,-1));
   polyList.mPlaneList[5].set(B.maxExtents, VectorF(0,0,1));
   //result = ! lContainer->buildPolyList(B, mask, &polyList);
   result = ! lContainer->buildPolyList(PLC_Collision, B, mask, &polyList);

   return result;
}

// --------------------------------------------------------------------------------------------
inline bool boxEmpty(Point3F center, Point3F extent, U32 mask, bool onClient = false) 
{
	Box3F    B(center - (extent / 2.f), center + (extent / 2.f), true);
	return _boxEmpty(B, mask, onClient);
}
// --------------------------------------------------------------------------------------------
inline bool boxEmpty2(Box3F box, U32 mask, bool onClient = false)
{
	//Box3F    B(center - (extent / 2.f), center + (extent / 2.f), true);
	return _boxEmpty(box, mask, onClient);
}
// --------------------------------------------------------------------------------------------
inline bool boxEmptyDoubleExtent(Point3F center, Point3F extent, U32 mask, bool onClient = false) 
{
	Box3F    B(center - extent, center + extent , true);
	return _boxEmpty(B, mask, onClient);
}
// --------------------------------------------------------------------------------------------
inline bool boxEmptyPos(Point3F pos, Point3F extent, U32 mask, bool onClient = false) 
{
	Box3F    B(pos, pos + extent, true);
	return _boxEmpty(B, mask, onClient);
}
// --------------------------------------------------------------------------------------------
inline bool simpleBoxEmpty(Point3F center, bool onClient = false)
{
  U32 mask = StaticShapeObjectType  ;
//TGE:   U32 mask = StaticShapeObjectType  | StaticTSObjectType | InteriorObjectType;
  Point3F extent(1.f,1.f,1.f);
  return boxEmpty(center,extent,mask,onClient);

}
// --------------------------------------------------------------------------------------------
// 2023-02-25 i want to move close to a object but not directly
// 2D !!! 
inline Point3F getPositionCloseToDestination(Point3F from, Point3F to, F32 radius)
{
/// is behind!!    VectorF  lForwardVector = AIMath::Vector2D(from - to);
   VectorF  lForwardVector = AIMath::Vector2D(to - from);
   lForwardVector.normalize();
   U32 lsaveZ = to.z;
   to.z = 0.f;
   Point3F lResult = to - (lForwardVector * radius);
   lResult.z = lsaveZ + 0.5f; //unsafe ?!? - add 0.5
   return lResult;
}

// same as befor but with objects - if radius = 0 we use WorldSphere radius! 
inline Point3F getPositionCloseToObject(SceneObject *  fromObj, SceneObject*  toObj, F32 radius = 1.f)
{
   if (!fromObj || !toObj)
   {
      Con::errorf(" getPositionCloseToObject NULL pointer detected!! CHECK both exists and are based on GameBase!! - give you your current position!");
      return fromObj->getPosition();
   }
   
   //make no sense! 
   if (radius < 0.1f) {
      return toObj->getPosition();
   }

   return getPositionCloseToDestination(fromObj->getPosition(), toObj->getPosition(),radius);
   
}


// --------------------------------------------------------------------------------------------
inline bool checkLos(Point3F from, Point3F to, U32 lMask, bool onClient = false)
{
   SceneContainer * lContainer;
   if (onClient)
	   lContainer = &gClientContainer;
   else
	   lContainer = &gServerContainer;

	// adjust positions to check slightly above terrain
	from.z += 0.5f; 
	to.z   += 0.5f; 

	RayInfo dummy;
	

	// if we can't get from node to neighbour without colliding, it is untraversal
	if (lContainer->castRay(from, to, lMask , &dummy))
		return false;

	return true;
}
// --------------------------------------------------------------------------------------------
inline bool CheckObjectsLos(SceneObject* obj1, SceneObject* obj2, U32 lMask = StaticShapeObjectType, bool onClient = false) {

   if (!obj1 || !obj2)
      return false;
   Point3F lF, lT;
   obj1->getWorldBox().getCenter(&lF);
   obj2->getWorldBox().getCenter(&lT);

   return AIMath::checkLos(lF, lT, lMask, onClient);

}

// --------------------------------------------------------------------------------------------
inline bool simpleCheckLos(Point3F from, Point3F to, bool onClient = false)
{
  U32 mask = StaticShapeObjectType ;
//TGE:  U32 mask = StaticShapeObjectType  | StaticTSObjectType | InteriorObjectType;
  return checkLos(from,to,mask,onClient);

}
// --------------------------------------------------------------------------------------------
inline Point3F getVariPoint(Point3F location, F32 tolerance = 2.f)
{
   return Point3F(
      location.x + gRandGen.randF(tolerance * -1, tolerance)
      , location.y + gRandGen.randF(tolerance * -1, tolerance)
      , location.z
   );
}


//------------------------------------------------------------------------------
 // Vector / collision stuff
//------------------------------------------------------------------------------

//--------------- https://stackoverflow.com/questions/14176776/find-out-if-2-lines-intersect#14177062
// BETTER? https://stackoverflow.com/questions/34415671/intersection-of-a-line-with-a-line-segment-in-c#34416122

inline bool rayIntersect(Point2F& p1, Point2F& p2, Point2F& q1, Point2F& q2) {
   return (((q1.x - p1.x) * (p2.y - p1.y) - (q1.y - p1.y) * (p2.x - p1.x))
      * ((q2.x - p1.x) * (p2.y - p1.y) - (q2.y - p1.y) * (p2.x - p1.x)) < 0)
      &&
      (((p1.x - q1.x) * (q2.y - q1.y) - (p1.y - q1.y) * (q2.x - q1.x))
         * ((p2.x - q1.x) * (q2.y - q1.y) - (p2.y - q1.y) * (q2.x - q1.x)) < 0);
}

// OR http://alienryderflex.com/intersect/
//  public domain function by Darel Rex Finley, 2006
//  Determines the intersection point of the line defined by points A and B with the
//  line defined by points C and D.
//
//  Returns YES if the intersection point was found, and stores that point in X,Y.
//  Returns NO if there is no determinable intersection point, in which case X,Y will
//  be unmodified.

inline bool lineIntersection(
   Point2F A,
   Point2F B,
   Point2F C,
   Point2F D,
   Point2F& colPoint) {

   F32  distAB, theCos, theSin, newX, ABpos;

   //  Fail if either line is undefined.
   if (A.x == B.x && A.y == B.y || C.x == D.x && C.y == D.y) return false;

   //  Fail if the segments share an end-point.
   if (A.x == C.x && A.y == C.y || B.x == C.x && B.y == C.y
      || A.x == D.x && A.y == D.y || B.x == D.x && B.y == D.y) {
      return false;
   }

   //  (1) Translate the system so that point A is on the origin.
   B.x -= A.x; B.y -= A.y;
   C.x -= A.x; C.y -= A.y;
   D.x -= A.x; D.y -= A.y;

   //  Discover the length of segment A-B.
   distAB = mSqrt(B.x * B.x + B.y * B.y);

   //  (2) Rotate the system so that point B is on the positive X axis.
   theCos = B.x / distAB;
   theSin = B.y / distAB;
   newX = C.x * theCos + C.y * theSin;
   C.y = C.y * theCos - C.x * theSin; C.x = newX;
   newX = D.x * theCos + D.y * theSin;
   D.y = D.y * theCos - D.x * theSin; D.x = newX;

   //  Fail if segment C-D doesn't cross line A-B.
   if (C.y < 0. && D.y < 0. || C.y >= 0. && D.y >= 0.) return false;

   //  (3) Discover the position of the intersection point along line A-B.
   ABpos = D.x + (C.x - D.x) * D.y / (D.y - C.y);

   //  Fail if segment C-D crosses line A-B outside of segment A-B.
   if (ABpos<0. || ABpos>distAB) return false;

   //  (4) Apply the discovered position to line A-B in the original coordinate system.
   colPoint.x = A.x + ABpos * theCos;
   colPoint.y = A.y + ABpos * theSin;

   //  Success.
   return true;
}
// --------------------------------------------------------------------------------------------
inline F32 PointDistance(Point2F v1, Point2F v2) {

   Point2F v = v2 - v1;
   return mSqrt((v.x * v.x) + (v.y * v.y));
}




} //AIMath

#endif //_AIMATH_H_
