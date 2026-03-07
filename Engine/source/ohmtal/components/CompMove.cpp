/**
  CompMove

  @since 2024-01-10
  @author XXTH

  License at: ohmtal/misc/ohmtalMIT.h
  -----------------------------------------------------------------------------

  FIXME smoother rotation

  -----------------------------------------------------------------------------
  Example:
  ========
   MyShip.add(new CompMove());
   MyShip.CompMove.setMoveDestination("20 0 100",10,2.f);
  -----------------------------------------------------------------------------
  Path example:
  =============
      function MyShipClass::setPath(%this,%pathObj)
      {
         // validate CompMove
         if (!isObject(%this.CompMove))
         {
            error(%this.getId() SPC "no move component!!!!");
            return;
         }
         // weak validate path exists
         if (!isObject(%pathObj))
         {
            if (isObject(ShipPath1))
               %pathObj = ShipPath1;
            else {
               error(%this.getId() SPC "addPath but no path available");
               return;
            }
         }
         // lets go
         %this.CompMove.openPath();
         for (%i = 0; %i < %pathObj.getCount(); %i++)
         {
            %this.CompMove.addPathPoint(%pathObj.getObject(%i).getPosition());
         }
         %this.CompMove.closePath(true);
         %this.CompMove.startPath(20);
      }
  <<<< path example -----------------------------
   

*/

#include "scene/sceneObject.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "math/mathUtils.h"
#include "../ai/aiPath.h"

#include "CompBase.h"

//-----------------------------------------------------------------------------
// Header
//-----------------------------------------------------------------------------
class CompMove : public CompBase
{
   typedef CompBase Parent;
   DECLARE_CONOBJECT(CompMove);

   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

public:


   CompMove() :
      mSpeed(0.f)
      , mDirection(0.f, 0.f, 0.f)
      , mRotateObject(false)
      , mMoveToDestination(false)
      , mMoveToleranz(1.f)
      , mMoveSpeed(0.f)
      , mSlowDownFact(0.f)
      , mDestination(0.f, 0.f, 0.f)
      , mDropOnTerrain(false)
      , mTerrainOffsetZ(0.f)
      , mPath(NULL)
      , mPathOpen(false)

   {
      mIdent = StringTable->insert("CompMove");
   }

   ~CompMove()
   {
      SAFE_DELETE(mPath);
   }
  
   // UpdateInterface stuff: 
   virtual void advanceTime(F32 dt);

   bool setMoveDestination(Point3F lDest, F32 lSpeed, F32 slowdownFact);

   // path 
   bool openPath(bool looping = true);
   bool addPathPoint(Point3F position);
   bool closePath(bool splineIze);
   bool startPath(F32 lSpeed, F32 slowdownFact = 0.f);
   bool stopPath();
   bool continuePath();
   

protected:
   F32          mSpeed;
   Point3F      mDirection; //forward vector
   bool         mRotateObject; //by forwardVector
   //--- move to destination instead of direction ---
   bool        mMoveToDestination;
   F32         mMoveToleranz;
   F32         mMoveSpeed;
   F32        mSlowDownFact;
   Point3F     mDestination;
   // ... drop to terrain ...
   bool        mDropOnTerrain;
   F32         mTerrainOffsetZ;
   void dropToTerrain(Point3F& location);
   // ... path ...
   AIPath*     mPath;
   bool        mPathOpen;

   DECLARE_CALLBACK(void, onReachDestination, ());

};

//-----------------------------------------------------------------------------
// Source: 
//-----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(CompMove);

IMPLEMENT_CALLBACK(CompMove, onReachDestination, void, (), (),
   "@brief We reached the destination."
);


void CompMove::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("CompMove");
      addField("speed", TypeF32, Offset(mSpeed, CompMove), "move speed, is overwritten when using Path or moveDestination");
      addField("direction", TypePoint3F, Offset(mDirection, CompMove), "move direction");
      addField("rotateForward", TypeBool, Offset(mRotateObject, CompMove), "Rotate object by forward vector");
      addField("DropOnTerrain", TypeBool, Offset(mDropOnTerrain, CompMove), "Drop the object on terrain");
      addField("TerrainOffsetZ", TypeF32, Offset(mTerrainOffsetZ, CompMove), "When dropped on terrain, you can set an Z axis offset (default 0)");

  endGroup("CompMove");

}
//-----------------------------------------------------------------------------
void CompMove::advanceTime(F32 dt)
{
   if (isServerObject() && getMountParent() && mAbs(mSpeed) > 0.f && mDirection.len() > 0.f)
   {
      // added FeelerLen to toleranz if speed is too high
      F32 lFeelerLen = 0.f;
      mDirection.normalize();
      MatrixF mat = getMountParent()->getTransform();
      Point3F newPos = mat.getPosition() +  (mDirection * mSpeed * dt);
      lFeelerLen = mFabs(VectorF(mat.getPosition() - newPos).len());

      if (mDropOnTerrain)
         dropToTerrain(newPos);


      if (mRotateObject)
      {
         //orig works but no  smooth rotation
         //
         // tries to add the vector difference in parts
         // but same result. 
         // 
         // should use yaw/pitch/(roll?) but need more time
         // to understand this in 3d 
         mat = MathUtils::createOrientFromDir(mDirection);
      }

      //set new position
      mat.setPosition(newPos);

      //finally set transform
      getMountParent()->setTransform(mat);


      // --- are we already there ??? ----

      if (mMoveToDestination)
      {

         F32 lDiffDist = mFabs((getMountParent()->getPosition() - mDestination).len());
         if (mSlowDownFact > 0.f && lDiffDist <= (mMoveToleranz + lFeelerLen) * 10.f)
         {
            //too late this night ... lol ...
              mSpeed /= mSlowDownFact;
              if (mSpeed < 1.f)
                 mSpeed = 1.f;
         }
         if (lDiffDist <= ( mMoveToleranz + lFeelerLen))
         {
            if (mPath && mPath->initDone() && !mPath->isStopped() && mPath->haveWayPoint()
               && !mPath->inPause() && !mPath->finished()
               && mPath->setNextWayPoint())
            {
               setMoveDestination(mPath->getCurrentWayPoint()->pos, mMoveSpeed, mSlowDownFact);
            }
            else {
               mSpeed = 0.f;
               onReachDestination_callback();
            }
         }
      }
   } // if (isServerObject() && getMountParent() && mSpeed > 0.f && mDirection.len() > 0.f)
}
//-----------------------------------------------------------------------------
bool CompMove::setMoveDestination(Point3F lDest, F32 lSpeed, F32 slowdownFact)
{
   if (!getMountParent())
      return false;
   mSpeed = lSpeed;
   mMoveToDestination = true;
   mDestination = lDest;
   mSlowDownFact = slowdownFact;
   mDirection = lDest - getMountParent()->getPosition();
   mMoveSpeed = lSpeed;

   return true;
}
//-----------------------------------------------------------------------------
// PATH 
//-----------------------------------------------------------------------------
bool CompMove::openPath(bool looping)
{
   if (mPathOpen)
   {
      Con::errorf("Close the CompMove Path before you open it again !!!");
      return false;
   }
   if (mPath == NULL)
      mPath = new AIPath();
   else
      mPath->clearPoints();

   mPath->setLooping(looping);
   mPathOpen = true;

   return true;
}
//-----------------------------------------------------------------------------
bool CompMove::addPathPoint(Point3F position)
{
   if (!mPathOpen)
   {
      Con::errorf("CompMove::addPathPoint => Path not open!");
      return false;
   }

   mPath->addPoint(position);

   return true;
}
//-----------------------------------------------------------------------------
bool CompMove::closePath(bool splineIze)
{
   if (!mPathOpen)
   {
      Con::errorf("CompMove::closePath => Path not open!");
      return false;
   }

   mPathOpen = false;
   if (splineIze)
   {
      //fixme splinePrec parameter ?! is 0.1f as default
      mPath->splineize();
   }

   return true;
}
bool CompMove::startPath(F32 lSpeed, F32 slowdownFact)
{
   if (!getMountParent())
      return false;

   if (mPath == NULL || mPathOpen || !mPath->haveNodes())
   {
      Con::errorf("CompMove::startPath() => Path not prepared!! open/add/close");
      return false;

   }
   mPath->init(getMountParent()->getPosition());

   setMoveDestination(mPath->getCurrentWayPoint()->pos, lSpeed, slowdownFact);

   return true;
}
//-----------------------------------------------------------------------------
bool CompMove::stopPath()
{
   if (mPath == NULL || mPathOpen || !mPath->haveNodes())
   {
      Con::errorf("CompMove::stopPath => Path not prepared!! open/add/close");
      return false;

   }
   mPath->Stop();
   mSpeed = 0.f;
   return true;
}
bool CompMove::continuePath()
{
   if (mPath == NULL || mPathOpen || !mPath->haveNodes())
   {
      Con::errorf("CompMove::stopPath => Path not prepared!! open/add/close");
      return false;

   }
   mPath->Resume();
   mSpeed = mMoveSpeed;
   return true;
}
//-----------------------------------------------------------------------------
// drop the parent on the terrain 
//-----------------------------------------------------------------------------
void CompMove::dropToTerrain(Point3F& location)
{
   bool lIsGhost = isGhost();

   U32 conformMask = (TerrainObjectType | TerrainLikeObjectType);
   RayInfo surfaceInfo;

   Point3F above = Point3F(location.x, location.y, location.z + 1000.f);
   Point3F below = Point3F(location.x, location.y, location.z - 1000.f);

   bool found = false;
   if (!lIsGhost) {
      if (gServerContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }
   else {
      if (gClientContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }

   if (found) {
      location.z = surfaceInfo.point.z + mTerrainOffsetZ;
   }
}
//-----------------------------------------------------------------------------
DefineEngineMethod(CompMove, setMoveDestination, bool, (Point3F lDest, F32 lSpeed, F32 slowDownFact), (5.f,0.f),
   "(Point3F destination, F32 speed = 5.f, F32 slowDownFact = 0.f)"
   "Tells the parent object to move to the location provided."
)
{
   return object->setMoveDestination(lDest, lSpeed, slowDownFact);
}

DefineEngineMethod(CompMove, openPath, bool, (bool looping),(true),
   "open a new path"
)
{
   return object->openPath(looping);
}

DefineEngineMethod(CompMove, addPathPoint, bool, (Point3F pos), ,
   "add a point to an open path"
)
{
   return object->addPathPoint(pos);
}


DefineEngineMethod(CompMove, closePath, bool, (bool splineize),(false),
   "close the path, optional splineize"
)
{
   return object->closePath(splineize);
}

DefineEngineMethod(CompMove, startPath, bool, (F32 lSpeed, F32 slowDownFact), (0.f),
   "(F32 speed , F32 slowDownFact = 0.f)"
   "start the path, first it must be initilized"
)
{
   return object->startPath(lSpeed, slowDownFact);
}

DefineEngineMethod(CompMove, stopPath, bool, (), ,
   "stop the current path"
)
{
   return object->stopPath();
}

DefineEngineMethod(CompMove, continuePath, bool, (), ,
   "stop the current path"
)
{
   return object->continuePath();
}



